#include "src/kernel/batch.h"

void SetBatchSize(size_t column_batch_size, size_t row_batch_size) {
    kColumnBatchSize = column_batch_size;
    kRowBatchSize = row_batch_size;
}

Batch::Batch(size_t rows_number, std::shared_ptr<Schema> schema, std::vector<std::shared_ptr<Column>> columns) {
    if (schema->GetColumnsNumber() != columns.size()) {
        throw std::runtime_error("columns count mismatch :: Batch");
    }
    rows_number_ = rows_number;
    schema_ = schema;
    columns_ = std::move(columns);
}

size_t Batch::GetColumnsNumber() const {
    return schema_->GetColumnsNumber();
}

size_t Batch::GetRowsNumber() const {
    return rows_number_;
}

const std::string& Batch::GetName(size_t index) const {
    return schema_->GetName(index);
}

Type Batch::GetType(size_t index) const {
    return schema_->GetType(index);
}

size_t Batch::FindColumn(const std::string& name) const {
    return schema_->FindColumn(name);
}

std::shared_ptr<Column> Batch::GetColumn(size_t index) const {
    return columns_[index];
}

std::shared_ptr<Schema> Batch::GetSchema() const {
    return schema_;
}

std::vector<std::shared_ptr<Column>> Batch::MoveColumns() {
    return std::move(columns_);
}
