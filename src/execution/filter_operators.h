#pragma once

#include <memory>
#include <vector>
#include "src/execution/expressions.h"
#include "src/execution/operators.h"
#include "src/objects.h"

class FilterOperator : public Operator {
public:
    FilterOperator(std::shared_ptr<Operator> next, std::shared_ptr<Expression> predicate);
    std::shared_ptr<Batch> Next() override;

private:
    std::shared_ptr<Operator> next_;
    std::shared_ptr<Expression> predicate_;
    bool mask_computed_;
    std::shared_ptr<Column> mask_;
};
