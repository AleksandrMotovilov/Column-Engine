#pragma once

#include <memory>
#include <string>
#include <vector>
#include "src/execution/operators.h"
#include "src/kernel/reader_writer_clmn.h"

class ScanOperator : public Operator {
public:
    ScanOperator(const std::string& file_clmn, std::vector<std::string> needed_columns, bool full_matrix = false);
    std::shared_ptr<Batch> Next() override;

private:
    std::vector<std::string> needed_columns_;
    std::unique_ptr<ReaderClmn> reader_;
    bool full_matrix_;
};
