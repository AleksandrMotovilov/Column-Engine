#pragma once

#include <memory>
#include <string>
#include "src/execution/operator.h"
#include "src/convertion/reader_writer_csv.h"

class WriteOperator : public Operator {
public:
    WriteOperator(const std::string& file_csv, const std::string& file_schema, std::shared_ptr<Operator> next);
    std::shared_ptr<Batch> Next() override;

private:
    std::shared_ptr<Operator> next_;
    std::unique_ptr<WriterCsv> writer_;
    bool done_;
};
