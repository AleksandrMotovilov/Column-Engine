#pragma once

#include <memory>
#include <string>
#include <vector>
#include "src/execution/expressions.h"
#include "src/execution/operators.h"
#include "src/batch.h"

class ProjectOperator : public Operator {
public:
    ProjectOperator(std::shared_ptr<Operator> next, std::shared_ptr<Expression> expr, std::string col_name);
    std::shared_ptr<Batch> Next() override;

private:
    std::shared_ptr<Operator> next_;
    std::shared_ptr<Expression> expr_;
    std::string column_name_;
    bool expr_evaluated_;
};
