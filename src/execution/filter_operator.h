#pragma once

#include <memory>
#include <vector>
#include "src/execution/expressions.h"
#include "src/execution/operator.h"
#include "src/kernel/batch.h"
#include "src/kernel/column_utils.h"

class FilterOperator : public Operator {
public:
    FilterOperator(std::shared_ptr<Operator> next, std::shared_ptr<Expression> predicate);
    std::shared_ptr<Batch> Next() override;

private:
    std::shared_ptr<Operator> next_;
    std::shared_ptr<Expression> predicate_;
};
