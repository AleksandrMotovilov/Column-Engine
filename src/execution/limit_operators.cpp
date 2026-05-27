#include "src/execution/limit_operators.h"

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
    std::vector<Type> columns_types = batch->GetTypes();
    std::vector<std::string> columns_names = batch->GetNames();
    std::shared_ptr<Batch> result = std::make_shared<Batch>(rows, batch->GetColumnsNumber(), columns_types, columns_names);
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < batch->GetColumnsNumber(); j++) {
            result->SetValue(i, j, batch->GetValue(i, j));
        }
    }
    return result;
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
    size_t rows = batch->GetRowsNumber();
    size_t start = std::min(offset_, rows);
    size_t new_rows = rows - start;
    if (start == 0) {
        return batch;
    }
    std::vector<Type> columns_types = batch->GetTypes();
    std::vector<std::string> columns_names = batch->GetNames();
    std::shared_ptr<Batch> result = std::make_shared<Batch>(new_rows, batch->GetColumnsNumber(), columns_types, columns_names);
    for (size_t i = 0; i < new_rows; i++) {
        for (size_t j = 0; j < batch->GetColumnsNumber(); j++) {
            result->SetValue(i, j, batch->GetValue(start + i, j));
        }
    }
    return result;
}
