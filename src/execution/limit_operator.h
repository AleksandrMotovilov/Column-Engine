#pragma once

#include <algorithm>
#include <numeric>
#include <memory>
#include <vector>
#include "src/execution/operator.h"
#include "src/kernel/batch.h"
#include "src/kernel/column_utils.h"

class LimitOperator : public Operator {
public:
    LimitOperator(std::shared_ptr<Operator> next, size_t limit);
    std::shared_ptr<Batch> Next() override;

private:
    std::shared_ptr<Operator> next_;
    size_t limit_;
};

class OffsetOperator : public Operator {
public:
    OffsetOperator(std::shared_ptr<Operator> next, size_t offset);
    std::shared_ptr<Batch> Next() override;

private:
    std::shared_ptr<Operator> next_;
    size_t offset_;
};
