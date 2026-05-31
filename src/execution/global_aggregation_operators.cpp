#include "src/execution/global_aggregation_operators.h"
#include "src/kernel/column_utils.h"

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

    bool got_batch = false;
    while (std::shared_ptr<Batch> batch = next_->Next()) {
        got_batch = true;
        for (std::shared_ptr<AggregationFunction>& aggregation : aggregations_) {
            aggregation->Update(batch);
        }
    }

    if (!got_batch) {
        return nullptr;
    }

    std::vector<Type> result_types;
    std::vector<std::string> result_names;
    std::vector<std::shared_ptr<Column>> result_columns;
    for (std::shared_ptr<AggregationFunction>& aggregation : aggregations_) {
        Type type = aggregation->GetType();
        result_types.push_back(type);
        result_names.push_back(aggregation->GetName());
        result_columns.push_back(MakeSingleValueColumn(type, aggregation->GetResult()));
    }
    return std::make_shared<Batch>(1, std::make_shared<Schema>(std::move(result_names), std::move(result_types)), std::move(result_columns));
}
