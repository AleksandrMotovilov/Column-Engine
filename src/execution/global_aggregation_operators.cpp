#include "src/execution/global_aggregation_operators.h"

GlobalAggregationOperator::GlobalAggregationOperator(std::shared_ptr<Operator> next, std::vector<std::shared_ptr<AggregationFunction>> aggregations) {
    next_ = std::move(next);
    aggregations_ = std::move(aggregations);
    done_ = false;
}

std::shared_ptr<Batch> GlobalAggregationOperator::Next() {
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
        for (std::shared_ptr<AggregationFunction>& aggregation : aggregations_) {
            aggregation->Update(merged);
        }
        
        std::vector<Type> columns_types;
        std::vector<std::string> columns_names;
        for (std::shared_ptr<AggregationFunction>& aggregation : aggregations_) {
            columns_types.push_back(aggregation->GetType());
            columns_names.push_back(aggregation->GetName());
        }

        std::shared_ptr<Batch> result = std::make_shared<Batch>(1, aggregations_.size(), columns_types, columns_names);
        for (size_t i = 0; i < aggregations_.size(); i++) {
            result->SetValue(0, i, aggregations_[i]->GetResult());
        }
        return result;
    }
    return nullptr;
}
