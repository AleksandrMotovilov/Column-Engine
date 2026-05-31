#pragma once

#include <memory>
#include <string>
#include "src/execution/operators.h"
#include "src/kernel/reader_writer_clmn.h"

class WriteOperator : public Operator {
public:
    WriteOperator(const std::string& file_clmn, std::shared_ptr<Operator> next);
    std::shared_ptr<Batch> Next() override;

private:
    std::shared_ptr<Operator> next_;
    std::unique_ptr<WriterClmn> writer_;
    bool done_;
};
