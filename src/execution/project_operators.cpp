#include "src/execution/project_operators.h"

ProjectOperator::ProjectOperator(std::shared_ptr<Operator> next, std::shared_ptr<Expression> expr, std::string column_name, Type column_type) {
    next_ = std::move(next);
    expr_ = std::move(expr);
    column_name_ = std::move(column_name);
    column_type_ = column_type;
    expr_evaluated_ = false;
}

std::shared_ptr<Batch> ProjectOperator::Next() {
    std::shared_ptr<Batch> batch = next_->Next();
    if (batch == nullptr) {
        return nullptr;
    }
    if (expr_evaluated_) {
        return batch;
    }
    expr_evaluated_ = true;
    std::shared_ptr<Column> computed = expr_->Eval(batch);
    size_t rows_number = batch->GetRowsNumber();
    size_t columns_number = batch->GetColumnsNumber();
    std::vector<Type> columns_types = batch->GetTypes();
    std::vector<std::string> columns_names = batch->GetNames();
    columns_types.push_back(column_type_);
    columns_names.push_back(column_name_);
    std::shared_ptr<Batch> result = std::make_shared<Batch>(rows_number, columns_number + 1, columns_types, columns_names);
    for (size_t i = 0; i < rows_number; i++) {
        for (size_t j = 0; j < columns_number; j++) {
            result->SetValue(i, j, batch->GetValue(i, j));
        }
        result->SetValue(i, columns_number, computed->GetValue(i));
    }
    return result;
}
