#include "src/execution/global_aggregation_operators.h"
#include "src/column.h"

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
    std::vector<std::shared_ptr<Column>> result_cols;
    for (std::shared_ptr<AggregationFunction>& aggregation : aggregations_) {
        Type t = aggregation->GetType();
        result_types.push_back(t);
        result_names.push_back(aggregation->GetName());
        result_cols.push_back(MakeSingleValueColumn(t, aggregation->GetResult()));
    }
    return std::make_shared<Batch>(1, std::move(result_names), std::move(result_types), std::move(result_cols));
}
