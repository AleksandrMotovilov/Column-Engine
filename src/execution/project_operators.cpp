#include "src/execution/project_operators.h"

ProjectOperator::ProjectOperator(std::shared_ptr<Operator> next, std::shared_ptr<Expression> expr, std::string column_name) {
    next_ = std::move(next);
    expr_ = std::move(expr);
    column_name_ = std::move(column_name);
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
    std::vector<Type> columns_types = batch->GetTypes();
    std::vector<std::string> columns_names = batch->GetNames();
    std::vector<std::shared_ptr<Column>> columns = batch->MoveColumns();
    Type column_type = expr_->GetType();
    columns_types.push_back(column_type);
    columns_names.push_back(column_name_);
    columns.push_back(std::move(computed));
    return std::make_shared<Batch>(rows_number, std::move(columns_names), std::move(columns_types), std::move(columns));
}
