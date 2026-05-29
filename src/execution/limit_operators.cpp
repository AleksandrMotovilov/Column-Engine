#include "src/execution/limit_operators.h"
#include "src/column.h"

LimitOperator::LimitOperator(std::shared_ptr<Operator> next, size_t limit) {
    next_ = std::move(next);
    limit_ = limit;
}

std::shared_ptr<Batch> LimitOperator::Next() {
    std::shared_ptr<Batch> batch = next_->Next();
    if (batch == nullptr) {
        return nullptr;
    }
    size_t rows = std::min(limit_, batch->GetRowsNumber());
    if (rows == batch->GetRowsNumber()) {
        return batch;
    }
    std::vector<size_t> indices(rows);
    std::iota(indices.begin(), indices.end(), 0);
    std::vector<Type> columns_types = batch->GetTypes();
    std::vector<std::string> columns_names = batch->GetNames();
    size_t columns_number = batch->GetColumnsNumber();
    std::vector<std::shared_ptr<Column>> columns;
    for (size_t i = 0; i < columns_number; i++) {
        columns.push_back(CopyRowsTyped(batch->GetColumn(i), columns_types[i], indices));
    }
    return std::make_shared<Batch>(rows, std::move(columns_names), std::move(columns_types), std::move(columns));
}

OffsetOperator::OffsetOperator(std::shared_ptr<Operator> next, size_t offset) {
    next_ = std::move(next);
    offset_ = offset;
}

std::shared_ptr<Batch> OffsetOperator::Next() {
    std::shared_ptr<Batch> batch = next_->Next();
    if (batch == nullptr) {
        return nullptr;
    }
    size_t total_rows = batch->GetRowsNumber();
    size_t start = std::min(offset_, total_rows);
    size_t rows = total_rows - start;
    if (start == 0) {
        return batch;
    }
    std::vector<size_t> indices(rows);
    std::iota(indices.begin(), indices.end(), start);
    std::vector<Type> columns_types = batch->GetTypes();
    std::vector<std::string> columns_names = batch->GetNames();
    size_t columns_number = batch->GetColumnsNumber();
    std::vector<std::shared_ptr<Column>> columns;
    for (size_t i = 0; i < columns_number; i++) {
        columns.push_back(CopyRowsTyped(batch->GetColumn(i), columns_types[i], indices));
    }
    return std::make_shared<Batch>(rows, std::move(columns_names), std::move(columns_types), std::move(columns));
}
