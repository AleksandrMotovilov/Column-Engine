#pragma once

#include <memory>
#include "src/kernel/batch.h"

class Operator {
public:
    virtual ~Operator() = default;
    virtual std::shared_ptr<Batch> Next() = 0;
};

#include "src/execution/scan_operators.h"
#include "src/execution/write_operators.h"
#include "src/execution/filter_operators.h"
#include "src/execution/project_operators.h"
#include "src/execution/global_aggregation_operators.h"
#include "src/execution/group_by_aggregation_operators.h"
#include "src/execution/top_k_operators.h"
#include "src/execution/sort_operators.h"
#include "src/execution/limit_operators.h"
