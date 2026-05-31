#pragma once

#include <algorithm>
#include <functional>
#include <unordered_map>
#include <memory>
#include <vector>
#include "src/execution/aggregation_functions.h"
#include "src/execution/operators.h"
#include "src/kernel/batch.h"

class GroupByAggregationOperator : public Operator {
public:
    using AggregationFactory = std::function<std::vector<std::shared_ptr<AggregationFunction>>()>;

    GroupByAggregationOperator(std::shared_ptr<Operator> next, std::vector<std::string> group_by_columns, AggregationFactory aggregation_factory);
    std::shared_ptr<Batch> Next() override;

private:
    std::shared_ptr<Operator> next_;
    std::vector<std::string> group_by_columns_;
    AggregationFactory aggregation_factory_;
    bool done_;
};
