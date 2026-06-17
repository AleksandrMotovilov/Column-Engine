#pragma once

#include <memory>
#include "src/kernel/batch.h"

class Operator {
public:
    virtual ~Operator() = default;
    virtual std::shared_ptr<Batch> Next() = 0;
};
