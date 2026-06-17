#include "src/execution/project_operators.h"

ProjectOperator::ProjectOperator(std::shared_ptr<Operator> next, std::shared_ptr<Expression> expr, std::string column_name) {
    next_ = std::move(next);
    expr_ = std::move(expr);
    column_name_ = std::move(column_name);
    schema_ = nullptr;
}

std::shared_ptr<Batch> ProjectOperator::Next() {
    std::shared_ptr<Batch> batch = next_->Next();
    if (batch == nullptr) {
        return nullptr;
    }
    std::shared_ptr<Column> computed = expr_->Eval(batch);
    size_t rows_number = batch->GetRowsNumber();
    if (schema_ == nullptr) {
        std::vector<std::string> columns_names = batch->GetSchema()->GetNames();
        std::vector<Type> columns_types = batch->GetSchema()->GetTypes();
        columns_names.push_back(column_name_);
        columns_types.push_back(expr_->GetType());
        schema_ = std::make_shared<Schema>(std::move(columns_names), std::move(columns_types));
    }
    std::vector<std::shared_ptr<Column>> columns = batch->MoveColumns();
    columns.push_back(std::move(computed));
    return std::make_shared<Batch>(rows_number, schema_, std::move(columns));
}
