#include "src/execution/scan_operator.h"

ScanOperator::ScanOperator(const std::string& file_clmn, std::optional<std::vector<std::string>> columns_names) {
    reader_ = std::make_unique<ReaderClmn>(file_clmn, std::move(columns_names));
    reader_->ReadSchema();
}

std::shared_ptr<Batch> ScanOperator::Next() {
    return reader_->ReadBatch();
}
