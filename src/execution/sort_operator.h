#pragma once

#include <algorithm>
#include <memory>
#include <numeric>
#include <vector>
#include "src/execution/operator.h"
#include "src/kernel/batch.h"
#include "src/kernel/column_utils.h"

class SortOperator : public Operator {
public:
    SortOperator(std::shared_ptr<Operator> next, std::string sort_column, bool descending);
    std::shared_ptr<Batch> Next() override;

private:
    std::shared_ptr<Operator> next_;
    std::string sort_column_;
    bool descending_;
    bool done_;
};
