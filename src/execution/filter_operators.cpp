#include "src/execution/filter_operators.h"
#include "src/column.h"

FilterOperator::FilterOperator(std::shared_ptr<Operator> next, std::shared_ptr<Expression> predicate) {
    next_ = std::move(next);
    predicate_ = std::move(predicate);
    mask_computed_ = false;
}

std::shared_ptr<Batch> FilterOperator::Next() {
    std::shared_ptr<Batch> batch = next_->Next();
    if (batch == nullptr) {
        return nullptr;
    }
    if (!mask_computed_) {
        mask_computed_ = true;
        std::shared_ptr<Column> mask_column = predicate_->Eval(batch);
        mask_ = dynamic_cast<ColumnTyped<char>&>(*mask_column).GetVector();
    }
    std::vector<size_t> indices;
    indices.reserve(mask_.size());
    for (size_t j = 0; j < mask_.size(); j++) {
        if (mask_[j] != '0') {
            indices.push_back(j);
        }
    }
    std::vector<Type> columns_types = batch->GetTypes();
    std::vector<std::string> columns_names = batch->GetNames();
    size_t columns_number = batch->GetColumnsNumber();
    std::vector<std::shared_ptr<Column>> columns;
    for (size_t i = 0; i < columns_number; i++) {
        columns.push_back(CopyRowsTyped(batch->GetColumn(i), columns_types[i], indices));
    }
    size_t count = indices.size();
    return std::make_shared<Batch>(count, std::move(columns_names), std::move(columns_types), std::move(columns));
}
