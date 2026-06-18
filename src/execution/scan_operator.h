#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "src/execution/operator.h"
#include "src/convertion/reader_writer_clmn.h"

class ScanOperator : public Operator {
public:
    ScanOperator(const std::string& file_clmn, std::optional<std::vector<std::string>> columns_names);
    std::shared_ptr<Batch> Next() override;

private:
    std::unique_ptr<ReaderClmn> reader_;
};
