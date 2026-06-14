#pragma once

#include <algorithm>
#include <memory>
#include <vector>
#include "src/execution/aggregation_functions.h"
#include "src/execution/operators.h"
#include "src/kernel/batch.h"
#include "src/kernel/column_utils.h"

class GlobalAggregationOperator : public Operator {
public:
    GlobalAggregationOperator(std::shared_ptr<Operator> next, std::vector<std::shared_ptr<AggregationFunction>> aggregations);
    std::shared_ptr<Batch> Next() override;

private:
    std::shared_ptr<Operator> next_;
    std::vector<std::shared_ptr<AggregationFunction>> aggregations_;
    bool done_;
};
