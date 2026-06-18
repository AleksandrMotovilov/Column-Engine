#pragma once

#include <algorithm>
#include <memory>
#include <numeric>
#include <vector>
#include "src/execution/operator.h"
#include "src/kernel/batch.h"
#include "src/kernel/column_utils.h"

std::shared_ptr<Batch> TopKFromBatch(std::shared_ptr<Batch> batch, size_t k, const std::vector<std::pair<size_t, Type>>& sort_keys, bool descending);

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
