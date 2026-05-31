#include "src/execution/group_by_aggregation_operators.h"
#include "src/kernel/column_utils.h"

struct VectorStringHash {
    size_t operator()(const std::vector<std::string>& v) const
    {
        size_t seed = v.size();
        for (const std::string& s : v) {
            seed ^= std::hash<std::string>{}(s) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

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

    std::unordered_map<std::vector<std::string>, size_t, VectorStringHash> key_to_index;
    std::vector<std::vector<std::string>> keys;
    std::vector<std::vector<size_t>> group_row_indices;
    for (size_t row = 0; row < merged->GetRowsNumber(); row++) {
        std::vector<std::string> key;
        for (size_t i = 0; i < group_column_indices.size(); i++) {
            key.push_back(GetStringValueAt(merged->GetColumn(group_column_indices[i]), group_column_types[i], row));
        }
        auto it = key_to_index.find(key);
        size_t index;
        if (it == key_to_index.end()) {
            index = keys.size();
            key_to_index[key] = index;
            keys.push_back(key);
            group_row_indices.push_back({});
        } else {
            index = it->second;
        }
        group_row_indices[index].push_back(row);
    }

    if (keys.empty()) {
        return nullptr;
    }

    size_t result_rows = keys.size();
    size_t sub_columns_count = merged_schema->GetColumnsNumber();
    std::vector<std::vector<std::string>> aggregation_results(result_rows);
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
            aggregation_results[group_index].push_back(agg->GetResult());
        }
        if (aggregation_types.empty()) {
            for (std::shared_ptr<AggregationFunction>& agg : aggregations) {
                aggregation_types.push_back(agg->GetType());
                aggregation_names.push_back(agg->GetName());
            }
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

    size_t total_columns_number = result_names.size();
    std::vector<std::vector<std::string>> column_strings(total_columns_number);
    for (size_t group_index = 0; group_index < result_rows; group_index++) {
        for (size_t i = 0; i < group_by_columns_.size(); i++) {
            column_strings[i].push_back(keys[group_index][i]);
        }
        for (size_t i = 0; i < aggregation_names.size(); i++) {
            column_strings[group_by_columns_.size() + i].push_back(aggregation_results[group_index][i]);
        }
    }

    std::vector<std::shared_ptr<Column>> result_columns;
    for (size_t i = 0; i < total_columns_number; i++) {
        result_columns.push_back(MakeColumnFromStrings(result_types[i], column_strings[i]));
    }

    return std::make_shared<Batch>(result_rows, std::make_shared<Schema>(std::move(result_names), std::move(result_types)), std::move(result_columns));
}
