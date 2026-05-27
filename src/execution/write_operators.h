#pragma once

#include <fstream>
#include <memory>
#include <stdexcept>
#include <vector>
#include "src/execution/operators.h"
#include "src/objects.h"

class WriteOperator : public Operator {
public:
    WriteOperator(const std::string& file_clmn, std::shared_ptr<Operator> next);
    std::shared_ptr<Batch> Next() override;

private:
    std::ofstream fout_;
    std::shared_ptr<Operator> next_;
};
