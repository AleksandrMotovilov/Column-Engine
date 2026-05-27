#include "src/execution/filter_operators.h"

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
        mask_ = predicate_->Eval(batch);
    }
    size_t count = 0;
    for (size_t i = 0; i < batch->GetRowsNumber(); i++) {
        if (mask_->GetValue(i) != "0") {
            count++;
        }
    }
    std::vector<Type> columns_types = batch->GetTypes();
    std::vector<std::string> columns_names = batch->GetNames();
    std::shared_ptr<Batch> result = std::make_shared<Batch>(count, batch->GetColumnsNumber(), columns_types, columns_names);
    size_t row = 0;
    for (size_t i = 0; i < batch->GetRowsNumber(); i++) {
        if (mask_->GetValue(i) != "0") {
            for (size_t j = 0; j < batch->GetColumnsNumber(); j++) {
                result->SetValue(row, j, batch->GetValue(i, j));
            }
            row++;
        }
    }
    return result;
}
