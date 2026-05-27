#include "src/execution/group_by_aggregation_operators.h"

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

    std::shared_ptr<Batch> merged;

    while (std::shared_ptr<Batch> batch = next_->Next()) {
        if (merged == nullptr) {
            merged = batch;
            continue;
        }

        size_t total_columns = merged->GetColumnsNumber() + batch->GetColumnsNumber();

        std::vector<Type> columns_types = merged->GetTypes();
        std::vector<Type> batch_types = batch->GetTypes();
        columns_types.insert(columns_types.end(), batch_types.begin(), batch_types.end());

        std::vector<std::string> columns_names = merged->GetNames();
        std::vector<std::string> batch_names = batch->GetNames();
        columns_names.insert(columns_names.end(), batch_names.begin(), batch_names.end());

        std::shared_ptr<Batch> combined = std::make_shared<Batch>(merged->GetRowsNumber(), total_columns, columns_types, columns_names);
        
        for (size_t i = 0; i < merged->GetRowsNumber(); i++) {
            for (size_t j = 0; j < merged->GetColumnsNumber(); j++) {
                combined->SetValue(i, j, merged->GetValue(i, j));
            }
            for (size_t j = 0; j < batch->GetColumnsNumber(); j++) {
                combined->SetValue(i, merged->GetColumnsNumber() + j, batch->GetValue(i, j));
            }
        }
        merged = combined;
    }

    if (merged != nullptr) {
        std::vector<std::string> columns_names = merged->GetNames();
        std::vector<Type> columns_types = merged->GetTypes();
        std::vector<size_t> group_column_indices;
        std::vector<Type> group_column_types;
        for (const std::string& gc : group_by_columns_) {
            for (size_t i = 0; i < columns_names.size(); i++) {
                if (columns_names[i] == gc) {
                    group_column_indices.push_back(i);
                    group_column_types.push_back(columns_types[i]);
                    break;
                }
            }
        }

        std::map<std::vector<std::string>, size_t> key_to_index;
        std::vector<std::vector<std::string>> keys;
        std::vector<std::vector<size_t>> group_row_indices;
        for (size_t row = 0; row < merged->GetRowsNumber(); row++) {
            std::vector<std::string> key;
            for (size_t gi : group_column_indices) {
                key.push_back(merged->GetValue(row, gi));
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

        if (!keys.empty()) {
            size_t result_rows = keys.size();
            size_t sub_cols = columns_names.size();
            std::vector<std::vector<std::string>> agg_results(result_rows);
            std::vector<Type> agg_types;
            std::vector<std::string> agg_names;
            for (size_t group_index = 0; group_index < result_rows; group_index++) {
                const std::vector<size_t>& row_indices = group_row_indices[group_index];
                size_t sub_rows = row_indices.size();
                std::shared_ptr<Batch> sub = std::make_shared<Batch>(sub_rows, sub_cols, columns_types, columns_names);
                for (size_t i = 0; i < sub_rows; i++) {
                    for (size_t j = 0; j < sub_cols; j++) {
                        sub->SetValue(i, j, merged->GetValue(row_indices[i], j));
                    }
                }
                std::vector<std::shared_ptr<AggregationFunction>> aggregations = aggregation_factory_();
                for (std::shared_ptr<AggregationFunction>& agg : aggregations) {
                    agg->Update(sub);
                    agg_results[group_index].push_back(agg->GetResult());
                }
                if (agg_types.empty()) {
                    for (std::shared_ptr<AggregationFunction>& agg : aggregations) {
                        agg_types.push_back(agg->GetType());
                        agg_names.push_back(agg->GetName());
                    }
                }
            }

            std::vector<Type> result_types;
            std::vector<std::string> result_names;
            for (size_t i = 0; i < group_by_columns_.size(); i++) {
                result_types.push_back(group_column_types[i]);
                result_names.push_back(group_by_columns_[i]);
            }
            result_types.insert(result_types.end(), agg_types.begin(), agg_types.end());
            result_names.insert(result_names.end(), agg_names.begin(), agg_names.end());
            
            std::shared_ptr<Batch> result = std::make_shared<Batch>(result_rows, result_types.size(), result_types, result_names);
            for (size_t group_index = 0; group_index < result_rows; group_index++) {
                size_t column = 0;
                for (const std::string& key : keys[group_index]) {
                    result->SetValue(group_index, column++, key);
                }
                for (const std::string& val : agg_results[group_index]) {
                    result->SetValue(group_index, column++, val);
                }
            }
            return result;
        }
    }
    return nullptr;
}
