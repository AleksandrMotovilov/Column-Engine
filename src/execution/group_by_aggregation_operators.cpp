#include "src/execution/group_by_aggregation_operators.h"
#include "src/column.h"

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

    std::vector<std::shared_ptr<Column>> all_columns;
    std::vector<std::string> all_names;
    std::vector<Type> all_types;
    size_t rows = 0;

    while (std::shared_ptr<Batch> batch = next_->Next()) {
        if (rows == 0) {
            rows = batch->GetRowsNumber();
        }
        std::vector<std::string> names = batch->GetNames();
        std::vector<Type> types = batch->GetTypes();
        all_names.insert(all_names.end(), names.begin(), names.end());
        all_types.insert(all_types.end(), types.begin(), types.end());
        std::vector<std::shared_ptr<Column>> columns = batch->MoveColumns();
        for (std::shared_ptr<Column>& column : columns) {
            all_columns.push_back(std::move(column));
        }
    }

    if (all_names.empty()) {
        return nullptr;
    }

    std::shared_ptr<Batch> merged = std::make_shared<Batch>(rows, std::move(all_names), std::move(all_types), std::move(all_columns));

    std::vector<std::string> columns_names = merged->GetNames();
    std::vector<Type> columns_types = merged->GetTypes();

    std::vector<size_t> group_column_indices;
    std::vector<Type> group_column_types;
    for (const std::string& column_name : group_by_columns_) {
        size_t index = merged->FindColumn(column_name);
        group_column_indices.push_back(index);
        group_column_types.push_back(columns_types[index]);
    }

    std::map<std::vector<std::string>, size_t> key_to_index;
    std::vector<std::vector<std::string>> keys;
    std::vector<std::vector<size_t>> group_row_indices;
    for (size_t row = 0; row < merged->GetRowsNumber(); row++) {
        std::vector<std::string> key;
        for (size_t i = 0; i < group_column_indices.size(); i++) {
            key.push_back(GetStringValueAt(merged->GetColumn(group_column_indices[i]), group_column_types[i], row));
        }
        std::map<std::vector<std::string>, size_t>::iterator it = key_to_index.find(key);
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
    size_t sub_columns_count = columns_names.size();
    std::vector<std::vector<std::string>> aggregation_results(result_rows);
    std::vector<Type> aggregation_types;
    std::vector<std::string> aggregation_names;

    for (size_t group_index = 0; group_index < result_rows; group_index++) {
        const std::vector<size_t>& row_indices = group_row_indices[group_index];
        std::vector<std::shared_ptr<Column>> sub_columns_vec;
        for (size_t j = 0; j < sub_columns_count; j++) {
            sub_columns_vec.push_back(CopyRowsTyped(merged->GetColumn(j), columns_types[j], row_indices));
        }
        std::vector<std::string> sub_names = columns_names;
        std::vector<Type> sub_types = columns_types;
        std::shared_ptr<Batch> sub = std::make_shared<Batch>(row_indices.size(), std::move(sub_names), std::move(sub_types), std::move(sub_columns_vec));
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

    std::vector<Type> result_types;
    std::vector<std::string> result_names;
    for (size_t i = 0; i < group_by_columns_.size(); i++) {
        result_types.push_back(group_column_types[i]);
        result_names.push_back(group_by_columns_[i]);
    }
    result_types.insert(result_types.end(), aggregation_types.begin(), aggregation_types.end());
    result_names.insert(result_names.end(), aggregation_names.begin(), aggregation_names.end());

    size_t total_cols = result_names.size();
    std::vector<std::vector<std::string>> column_strings(total_cols);
    for (size_t group_index = 0; group_index < result_rows; group_index++) {
        for (size_t i = 0; i < group_by_columns_.size(); i++) {
            column_strings[i].push_back(keys[group_index][i]);
        }
        for (size_t i = 0; i < aggregation_names.size(); i++) {
            column_strings[group_by_columns_.size() + i].push_back(aggregation_results[group_index][i]);
        }
    }

    std::vector<std::shared_ptr<Column>> result_columns;
    for (size_t i = 0; i < total_cols; i++) {
        result_columns.push_back(MakeColumnFromStrings(result_types[i], column_strings[i]));
    }

    return std::make_shared<Batch>(result_rows, std::move(result_names), std::move(result_types), std::move(result_columns));
}
