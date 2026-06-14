#include "src/execution/filter_operators.h"

FilterOperator::FilterOperator(std::shared_ptr<Operator> next, std::shared_ptr<Expression> predicate) {
    next_ = std::move(next);
    predicate_ = std::move(predicate);
}

std::shared_ptr<Batch> FilterOperator::Next() {
    std::shared_ptr<Batch> batch = next_->Next();
    if (batch == nullptr) {
        return nullptr;
    }
    std::shared_ptr<Column> mask_column = predicate_->Eval(batch);
    const std::vector<char>& mask = dynamic_cast<const ColumnTyped<char>&>(*mask_column).GetData();
    std::vector<size_t> indices;
    indices.reserve(mask.size());
    for (size_t j = 0; j < mask.size(); j++) {
        if (mask[j] != '0') {
            indices.push_back(j);
        }
    }
    std::shared_ptr<Schema> schema = batch->GetSchema();
    size_t columns_number = schema->GetColumnsNumber();
    std::vector<std::shared_ptr<Column>> columns;
    for (size_t i = 0; i < columns_number; i++) {
        columns.push_back(CopyRowsTyped(batch->GetColumn(i), schema->GetType(i), indices));
    }
    return std::make_shared<Batch>(indices.size(), schema, std::move(columns));
}
