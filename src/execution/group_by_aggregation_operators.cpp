#include "src/execution/group_by_aggregation_operators.h"

size_t VectorCharHash::operator()(const std::vector<char>& v) const {
    size_t seed = v.size();
    for (char c : v) {
        seed ^= std::hash<char>{}(c) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}

void AppendToKey(std::vector<char>& key, std::shared_ptr<Column> column, Type type, size_t index) {
    switch (type) {
        case Type::Int16: {
            int16_t value = dynamic_cast<const ColumnTyped<int16_t>&>(*column).GetData()[index];
            const char* ptr = reinterpret_cast<const char*>(&value);
            key.insert(key.end(), ptr, ptr + sizeof(int16_t));
            break;
        }
        case Type::Int32: {
            int32_t value = dynamic_cast<const ColumnTyped<int32_t>&>(*column).GetData()[index];
            const char* ptr = reinterpret_cast<const char*>(&value);
            key.insert(key.end(), ptr, ptr + sizeof(int32_t));
            break;
        }
        case Type::Int64: {
            int64_t value = dynamic_cast<const ColumnTyped<int64_t>&>(*column).GetData()[index];
            const char* ptr = reinterpret_cast<const char*>(&value);
            key.insert(key.end(), ptr, ptr + sizeof(int64_t));
            break;
        }
        case Type::Float: {
            float value = dynamic_cast<const ColumnTyped<float>&>(*column).GetData()[index];
            const char* ptr = reinterpret_cast<const char*>(&value);
            key.insert(key.end(), ptr, ptr + sizeof(float));
            break;
        }
        case Type::Double: {
            double value = dynamic_cast<const ColumnTyped<double>&>(*column).GetData()[index];
            const char* ptr = reinterpret_cast<const char*>(&value);
            key.insert(key.end(), ptr, ptr + sizeof(double));
            break;
        }
        case Type::Date: {
            int32_t value = dynamic_cast<const ColumnTyped<Date>&>(*column).GetData()[index].GetValue();
            const char* ptr = reinterpret_cast<const char*>(&value);
            key.insert(key.end(), ptr, ptr + sizeof(int32_t));
            break;
        }
        case Type::Timestamp: {
            int64_t value = dynamic_cast<const ColumnTyped<Timestamp>&>(*column).GetData()[index].GetValue();
            const char* ptr = reinterpret_cast<const char*>(&value);
            key.insert(key.end(), ptr, ptr + sizeof(int64_t));
            break;
        }
        case Type::Char: {
            char value = dynamic_cast<const ColumnTyped<char>&>(*column).GetData()[index];
            key.push_back(value);
            break;
        }
        case Type::String: {
            const std::string& s = dynamic_cast<const ColumnTyped<std::string>&>(*column).GetData()[index];
            size_t len = s.size();
            key.insert(key.end(), reinterpret_cast<const char*>(&len), reinterpret_cast<const char*>(&len) + sizeof(size_t));
            key.insert(key.end(), s.begin(), s.end());
            break;
        }
        default:
            throw std::runtime_error("Unsupported type :: AppendToKey");
    }
}

GroupByAggregationOperator::GroupByAggregationOperator(std::shared_ptr<Operator> next, std::vector<std::string> group_by_columns, AggregationFactory aggregation_factory) {
    next_ = std::move(next);
    group_by_columns_ = std::move(group_by_columns);
    aggregation_factory_ = std::move(aggregation_factory);
    done_ = false;
}

std::shared_ptr<Batch> GroupByAggregationOperator::Next() {
    if (done_) {
        return nullptr;
    }
    done_ = true;

    std::vector<std::shared_ptr<Batch>> all_batches;
    while (std::shared_ptr<Batch> batch = next_->Next()) {
        all_batches.push_back(batch);
    }

    if (all_batches.empty()) {
        return nullptr;
    }

    std::shared_ptr<Batch> merged = MergeBatchesByRows(all_batches);
    if (merged == nullptr) {
        return nullptr;
    }

    std::shared_ptr<Schema> merged_schema = merged->GetSchema();

    std::vector<size_t> group_column_indices;
    std::vector<Type> group_column_types;
    for (const std::string& column_name : group_by_columns_) {
        size_t index = merged_schema->FindColumn(column_name);
        group_column_indices.push_back(index);
        group_column_types.push_back(merged_schema->GetType(index));
    }

    std::unordered_map<std::vector<char>, size_t, VectorCharHash> key_to_index;
    std::vector<size_t> group_representative_rows;
    std::vector<std::vector<size_t>> group_row_indices;
    for (size_t row = 0; row < merged->GetRowsNumber(); row++) {
        std::vector<char> key;
        for (size_t i = 0; i < group_column_indices.size(); i++) {
            AppendToKey(key, merged->GetColumn(group_column_indices[i]), group_column_types[i], row);
        }
        auto it = key_to_index.find(key);
        size_t index;
        if (it == key_to_index.end()) {
            index = group_representative_rows.size();
            key_to_index[key] = index;
            group_representative_rows.push_back(row);
            group_row_indices.push_back({});
        } else {
            index = it->second;
        }
        group_row_indices[index].push_back(row);
    }

    if (group_representative_rows.empty()) {
        return nullptr;
    }

    size_t result_rows = group_representative_rows.size();
    size_t sub_columns_count = merged_schema->GetColumnsNumber();
    std::vector<std::vector<char>> agg_bufs;
    std::vector<Type> aggregation_types;
    std::vector<std::string> aggregation_names;

    for (size_t group_index = 0; group_index < result_rows; group_index++) {
        const std::vector<size_t>& row_indices = group_row_indices[group_index];
        std::vector<std::shared_ptr<Column>> sub_columns_vec;
        for (size_t j = 0; j < sub_columns_count; j++) {
            sub_columns_vec.push_back(CopyRowsTyped(merged->GetColumn(j), merged_schema->GetType(j), row_indices));
        }
        std::shared_ptr<Batch> sub = std::make_shared<Batch>(row_indices.size(), merged_schema, std::move(sub_columns_vec));
        std::vector<std::shared_ptr<AggregationFunction>> aggregations = aggregation_factory_();
        for (std::shared_ptr<AggregationFunction>& agg : aggregations) {
            agg->Update(sub);
        }
        if (agg_bufs.empty()) {
            agg_bufs.resize(aggregations.size());
            for (std::shared_ptr<AggregationFunction>& agg : aggregations) {
                aggregation_types.push_back(agg->GetType());
                aggregation_names.push_back(agg->GetName());
            }
        }
        for (size_t i = 0; i < aggregations.size(); i++) {
            aggregations[i]->AppendResultBytes(agg_bufs[i]);
        }
    }

    std::vector<std::string> result_names;
    std::vector<Type> result_types;
    for (size_t i = 0; i < group_by_columns_.size(); i++) {
        result_names.push_back(group_by_columns_[i]);
        result_types.push_back(group_column_types[i]);
    }
    result_names.insert(result_names.end(), aggregation_names.begin(), aggregation_names.end());
    result_types.insert(result_types.end(), aggregation_types.begin(), aggregation_types.end());

    std::vector<std::shared_ptr<Column>> result_columns;
    for (size_t i = 0; i < group_by_columns_.size(); i++) {
        result_columns.push_back(CopyRowsTyped(merged->GetColumn(group_column_indices[i]), group_column_types[i], group_representative_rows));
    }
    for (size_t i = 0; i < aggregation_names.size(); i++) {
        result_columns.push_back(MakeColumnFromBytes(agg_bufs[i], aggregation_types[i], result_rows));
    }

    return std::make_shared<Batch>(result_rows, std::make_shared<Schema>(std::move(result_names), std::move(result_types)), std::move(result_columns));
}
