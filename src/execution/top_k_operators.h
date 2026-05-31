#pragma once

#include <algorithm>
#include <memory>
#include <numeric>
#include <vector>
#include "src/execution/operators.h"
#include "src/kernel/batch.h"

class TopKOperator : public Operator {
public:
    TopKOperator(std::shared_ptr<Operator> next, std::vector<std::string> sort_columns, size_t limit, bool descending);
    std::shared_ptr<Batch> Next() override;

private:
    std::shared_ptr<Operator> next_;
    std::vector<std::string> sort_columns_;
    size_t limit_;
    bool descending_;
    bool done_;
};
