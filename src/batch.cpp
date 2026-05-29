#include "src/batch.h"

void SetBatchSize(size_t column_batch_size, size_t row_batch_size) {
    kColumnBatchSize = column_batch_size;
    kRowBatchSize = row_batch_size;
}

Batch::Batch(size_t rows_number, std::vector<std::string> names, std::vector<Type> types, std::vector<std::shared_ptr<Column>> columns) {
    if (types.size() != columns.size() || names.size() != columns.size()) {
        throw std::runtime_error("columns count mismatch :: Batch");
    }
    rows_number_ = rows_number;
    names_ = std::move(names);
    types_ = std::move(types);
    columns_ = std::move(columns);
}

size_t Batch::GetColumnsNumber() const {
    return columns_.size();
}

size_t Batch::GetRowsNumber() const {
    return rows_number_;
}

std::vector<Type> Batch::GetTypes() const {
    return types_;
}

std::vector<std::string> Batch::GetNames() const {
    return names_;
}

size_t Batch::FindColumn(const std::string& name) const {
    for (size_t i = 0; i < names_.size(); i++) {
        if (names_[i] == name) {
            return i;
        }
    }
    return names_.size();
}

std::shared_ptr<Column> Batch::GetColumn(size_t index) const {
    return columns_[index];
}

std::vector<std::shared_ptr<Column>> Batch::MoveColumns() {
    return std::move(columns_);
}
