#include "src/execution/scan_operators.h"

ScanOperator::ScanOperator(const std::string& file_clmn, std::vector<std::string> needed_columns, bool full_matrix) {
    needed_columns_ = std::move(needed_columns);
    full_matrix_ = full_matrix;
    reader_ = std::make_unique<ReaderClmn>(file_clmn);
    reader_->ReadSchema();
}

std::shared_ptr<Batch> ScanOperator::Next() {
    if (full_matrix_) {
        return reader_->ReadBatch();
    }
    return reader_->ReadBatchColumns(needed_columns_);
}
