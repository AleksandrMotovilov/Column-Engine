#include "src/execution/expressions.h"

EqualExpression::EqualExpression(std::string column_name, std::string value) {
    column_name_ = std::move(column_name);
    value_ = std::move(value);
}

std::shared_ptr<Column> EqualExpression::Eval(std::shared_ptr<Batch> batch) const {
    std::vector<std::string> columns_names = batch->GetNames();
    size_t column_index = columns_names.size();
    for (size_t i = 0; i < columns_names.size(); i++) {
        if (columns_names[i] == column_name_) {
            column_index = i;
        }
    }
    if (column_index == columns_names.size()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: EqualExpression");
    }
    size_t rows = batch->GetRowsNumber();
    std::shared_ptr<Column> result = std::make_shared<ColumnTyped<char>>(rows);
    for (size_t i = 0; i < rows; i++) {
        if (batch->GetValue(i, column_index) == value_) {
            result->SetValue(i, "1");
        } else {
            result->SetValue(i, "0");
        }
    }
    return result;
}

NotEqualExpression::NotEqualExpression(std::string column_name, std::string value) {
    column_name_ = std::move(column_name);
    value_ = std::move(value);
}

std::shared_ptr<Column> NotEqualExpression::Eval(std::shared_ptr<Batch> batch) const {
    std::vector<std::string> columns_names = batch->GetNames();
    size_t column_index = columns_names.size();
    for (size_t i = 0; i < columns_names.size(); i++) {
        if (columns_names[i] == column_name_) {
            column_index = i;
        }
    }
    if (column_index == columns_names.size()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: NotEqualExpression");
    }
    size_t rows = batch->GetRowsNumber();
    std::shared_ptr<Column> result = std::make_shared<ColumnTyped<char>>(rows);
    for (size_t i = 0; i < rows; i++) {
        if (batch->GetValue(i, column_index) != value_) {
            result->SetValue(i, "1");
        } else {
            result->SetValue(i, "0");
        }
    }
    return result;
}

GreaterOrEqualExpression::GreaterOrEqualExpression(std::string column_name, std::string value) {
    column_name_ = std::move(column_name);
    value_ = std::move(value);
}

std::shared_ptr<Column> GreaterOrEqualExpression::Eval(std::shared_ptr<Batch> batch) const {
    std::vector<std::string> columns_names = batch->GetNames();
    size_t column_index = columns_names.size();
    for (size_t i = 0; i < columns_names.size(); i++) {
        if (columns_names[i] == column_name_) {
            column_index = i;
        }
    }
    if (column_index == columns_names.size()) {
        throw std::runtime_error(
            "Column not found: " + column_name_ + " :: GreaterOrEqualExpression");
    }
    Type col_type = batch->GetTypes()[column_index];
    size_t rows = batch->GetRowsNumber();
    std::shared_ptr<Column> result = std::make_shared<ColumnTyped<char>>(rows);
    for (size_t i = 0; i < rows; i++) {
        const std::string& val = batch->GetValue(i, column_index);
        bool passes = false;
        switch (col_type) {
            case Type::Int16:     passes = FromString<int16_t>(val) >= FromString<int16_t>(value_); break;
            case Type::Int32:     passes = FromString<int32_t>(val) >= FromString<int32_t>(value_); break;
            case Type::Int64:     passes = FromString<int64_t>(val) >= FromString<int64_t>(value_); break;
            case Type::Float:     passes = FromString<float>(val) >= FromString<float>(value_); break;
            case Type::Double:    passes = FromString<double>(val) >= FromString<double>(value_); break;
            case Type::Date:
                passes = FromString<Date>(val).days >= FromString<Date>(value_).days; break;
            case Type::Timestamp:
                passes = FromString<Timestamp>(val).seconds >= FromString<Timestamp>(value_).seconds; break;
            default:              passes = val >= value_; break;
        }
        if (passes) {
            result->SetValue(i, "1");
        } else {
            result->SetValue(i, "0");
        }
    }
    return result;
}

LessOrEqualExpression::LessOrEqualExpression(std::string column_name, std::string value) {
    column_name_ = std::move(column_name);
    value_ = std::move(value);
}

std::shared_ptr<Column> LessOrEqualExpression::Eval(std::shared_ptr<Batch> batch) const {
    std::vector<std::string> columns_names = batch->GetNames();
    size_t column_index = columns_names.size();
    for (size_t i = 0; i < columns_names.size(); i++) {
        if (columns_names[i] == column_name_) {
            column_index = i;
        }
    }
    if (column_index == columns_names.size()) {
        throw std::runtime_error(
            "Column not found: " + column_name_ + " :: LessOrEqualExpression");
    }
    Type col_type = batch->GetTypes()[column_index];
    size_t rows = batch->GetRowsNumber();
    std::shared_ptr<Column> result = std::make_shared<ColumnTyped<char>>(rows);
    for (size_t i = 0; i < rows; i++) {
        const std::string& val = batch->GetValue(i, column_index);
        bool passes = false;
        switch (col_type) {
            case Type::Int16:     passes = FromString<int16_t>(val) <= FromString<int16_t>(value_); break;
            case Type::Int32:     passes = FromString<int32_t>(val) <= FromString<int32_t>(value_); break;
            case Type::Int64:     passes = FromString<int64_t>(val) <= FromString<int64_t>(value_); break;
            case Type::Float:     passes = FromString<float>(val) <= FromString<float>(value_); break;
            case Type::Double:    passes = FromString<double>(val) <= FromString<double>(value_); break;
            case Type::Date:
                passes = FromString<Date>(val).days <= FromString<Date>(value_).days; break;
            case Type::Timestamp:
                passes = FromString<Timestamp>(val).seconds <= FromString<Timestamp>(value_).seconds; break;
            default:              passes = val <= value_; break;
        }
        if (passes) {
            result->SetValue(i, "1");
        } else {
            result->SetValue(i, "0");
        }
    }
    return result;
}

ContainsExpression::ContainsExpression(std::string column_name, std::string substring) {
    column_name_ = std::move(column_name);
    substring_ = std::move(substring);
}

std::shared_ptr<Column> ContainsExpression::Eval(std::shared_ptr<Batch> batch) const {
    std::vector<std::string> columns_names = batch->GetNames();
    size_t column_index = columns_names.size();
    for (size_t i = 0; i < columns_names.size(); i++) {
        if (columns_names[i] == column_name_) {
            column_index = i;
        }
    }
    if (column_index == columns_names.size()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: ContainsExpression");
    }
    size_t rows = batch->GetRowsNumber();
    std::shared_ptr<Column> result = std::make_shared<ColumnTyped<char>>(rows);
    for (size_t i = 0; i < rows; i++) {
        if (batch->GetValue(i, column_index).find(substring_) != std::string::npos) {
            result->SetValue(i, "1");
        } else {
            result->SetValue(i, "0");
        }
    }
    return result;
}

NotContainsExpression::NotContainsExpression(std::string column_name, std::string substring) {
    column_name_ = std::move(column_name);
    substring_ = std::move(substring);
}

std::shared_ptr<Column> NotContainsExpression::Eval(std::shared_ptr<Batch> batch) const {
    std::vector<std::string> columns_names = batch->GetNames();
    size_t column_index = columns_names.size();
    for (size_t i = 0; i < columns_names.size(); i++) {
        if (columns_names[i] == column_name_) {
            column_index = i;
        }
    }
    if (column_index == columns_names.size()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: NotContainsExpression");
    }
    size_t rows = batch->GetRowsNumber();
    std::shared_ptr<Column> result = std::make_shared<ColumnTyped<char>>(rows);
    for (size_t i = 0; i < rows; i++) {
        if (batch->GetValue(i, column_index).find(substring_) == std::string::npos) {
            result->SetValue(i, "1");
        } else {
            result->SetValue(i, "0");
        }
    }
    return result;
}

AndExpression::AndExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right) {
    left_ = std::move(left);
    right_ = std::move(right);
}

std::shared_ptr<Column> AndExpression::Eval(std::shared_ptr<Batch> batch) const {
    std::shared_ptr<Column> left_mask = left_->Eval(batch);
    std::shared_ptr<Column> right_mask = right_->Eval(batch);
    size_t rows = batch->GetRowsNumber();
    std::shared_ptr<Column> result = std::make_shared<ColumnTyped<char>>(rows);
    for (size_t i = 0; i < rows; i++) {
        if (left_mask->GetValue(i) != "0" && right_mask->GetValue(i) != "0") {
            result->SetValue(i, "1");
        } else {
            result->SetValue(i, "0");
        }
    }
    return result;
}

OrExpression::OrExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right) {
    left_ = std::move(left);
    right_ = std::move(right);
}

std::shared_ptr<Column> OrExpression::Eval(std::shared_ptr<Batch> batch) const {
    std::shared_ptr<Column> left_mask = left_->Eval(batch);
    std::shared_ptr<Column> right_mask = right_->Eval(batch);
    size_t rows = batch->GetRowsNumber();
    std::shared_ptr<Column> result = std::make_shared<ColumnTyped<char>>(rows);
    for (size_t i = 0; i < rows; i++) {
        if (left_mask->GetValue(i) != "0" || right_mask->GetValue(i) != "0") {
            result->SetValue(i, "1");
        } else {
            result->SetValue(i, "0");
        }
    }
    return result;
}

ConstantExpression::ConstantExpression(std::string value) {
    value_ = std::move(value);
}

std::shared_ptr<Column> ConstantExpression::Eval(std::shared_ptr<Batch> batch) const {
    size_t rows = batch->GetRowsNumber();
    std::shared_ptr<Column> result = std::make_shared<ColumnTyped<std::string>>(rows);
    for (size_t i = 0; i < rows; i++) {
        result->SetValue(i, value_);
    }
    return result;
}

SumExpression::SumExpression(std::string column_name, int64_t constant) {
    column_name_ = std::move(column_name);
    constant_ = constant;
}

std::shared_ptr<Column> SumExpression::Eval(std::shared_ptr<Batch> batch) const {
    std::vector<std::string> columns_names = batch->GetNames();
    size_t column_index = columns_names.size();
    for (size_t i = 0; i < columns_names.size(); i++) {
        if (columns_names[i] == column_name_) {
            column_index = i;
        }
    }
    if (column_index == columns_names.size()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: SumExpression");
    }
    size_t rows = batch->GetRowsNumber();
    std::shared_ptr<Column> result = std::make_shared<ColumnTyped<int64_t>>(rows);
    for (size_t i = 0; i < rows; i++) {
        int64_t val = FromString<int64_t>(batch->GetValue(i, column_index));
        result->SetValue(i, ToString<int64_t>(val + constant_));
    }
    return result;
}

SubExpression::SubExpression(std::string column_name, int64_t constant) {
    column_name_ = std::move(column_name);
    constant_ = constant;
}

std::shared_ptr<Column> SubExpression::Eval(std::shared_ptr<Batch> batch) const {
    std::vector<std::string> columns_names = batch->GetNames();
    size_t column_index = columns_names.size();
    for (size_t i = 0; i < columns_names.size(); i++) {
        if (columns_names[i] == column_name_) {
            column_index = i;
        }
    }
    if (column_index == columns_names.size()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: SubExpression");
    }
    size_t rows = batch->GetRowsNumber();
    std::shared_ptr<Column> result = std::make_shared<ColumnTyped<int64_t>>(rows);
    for (size_t i = 0; i < rows; i++) {
        int64_t val = FromString<int64_t>(batch->GetValue(i, column_index));
        result->SetValue(i, ToString<int64_t>(val - constant_));
    }
    return result;
}

LengthExpression::LengthExpression(std::string column_name) {
    column_name_ = std::move(column_name);
}

std::shared_ptr<Column> LengthExpression::Eval(std::shared_ptr<Batch> batch) const {
    std::vector<std::string> columns_names = batch->GetNames();
    size_t column_index = columns_names.size();
    for (size_t i = 0; i < columns_names.size(); i++) {
        if (columns_names[i] == column_name_) {
            column_index = i;
        }
    }
    if (column_index == columns_names.size()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: LengthExpression");
    }
    size_t rows = batch->GetRowsNumber();
    std::shared_ptr<Column> result = std::make_shared<ColumnTyped<int64_t>>(rows);
    for (size_t i = 0; i < rows; i++) {
        result->SetValue(i, ToString<int64_t>(static_cast<int64_t>(batch->GetValue(i, column_index).size())));
    }
    return result;
}

RegexpReplaceExpression::RegexpReplaceExpression(
    std::string column_name, std::string pattern, std::string replacement) {
    column_name_ = std::move(column_name);
    pattern_ = std::regex(pattern);
    replacement_ = std::move(replacement);
}

std::shared_ptr<Column> RegexpReplaceExpression::Eval(std::shared_ptr<Batch> batch) const {
    std::vector<std::string> columns_names = batch->GetNames();
    size_t column_index = columns_names.size();
    for (size_t i = 0; i < columns_names.size(); i++) {
        if (columns_names[i] == column_name_) {
            column_index = i;
        }
    }
    if (column_index == columns_names.size()) {
        throw std::runtime_error(
            "Column not found: " + column_name_ + " :: RegexpReplaceExpression");
    }
    size_t rows = batch->GetRowsNumber();
    std::shared_ptr<Column> result = std::make_shared<ColumnTyped<std::string>>(rows);
    for (size_t i = 0; i < rows; i++) {
        result->SetValue(i, std::regex_replace(batch->GetValue(i, column_index), pattern_, replacement_));
    }
    return result;
}

CaseWhenExpression::CaseWhenExpression(
    std::shared_ptr<Expression> condition, std::string then_column, std::string else_value) {
    condition_ = std::move(condition);
    then_column_ = std::move(then_column);
    else_value_ = std::move(else_value);
}

std::shared_ptr<Column> CaseWhenExpression::Eval(std::shared_ptr<Batch> batch) const {
    std::shared_ptr<Column> mask = condition_->Eval(batch);
    std::vector<std::string> columns_names = batch->GetNames();
    size_t column_index = columns_names.size();
    for (size_t i = 0; i < columns_names.size(); i++) {
        if (columns_names[i] == then_column_) {
            column_index = i;
        }
    }
    if (column_index == columns_names.size()) {
        throw std::runtime_error("Column not found: " + then_column_ + " :: CaseWhenExpression");
    }
    size_t rows = batch->GetRowsNumber();
    std::shared_ptr<Column> result = std::make_shared<ColumnTyped<std::string>>(rows);
    for (size_t i = 0; i < rows; i++) {
        if (mask->GetValue(i) != "0") {
            result->SetValue(i, batch->GetValue(i, column_index));
        } else {
            result->SetValue(i, else_value_);
        }
    }
    return result;
}

TruncateToMinuteExpression::TruncateToMinuteExpression(std::string column_name) {
    column_name_ = std::move(column_name);
}

std::shared_ptr<Column> TruncateToMinuteExpression::Eval(std::shared_ptr<Batch> batch) const {
    std::vector<std::string> columns_names = batch->GetNames();
    size_t column_index = columns_names.size();
    for (size_t i = 0; i < columns_names.size(); i++) {
        if (columns_names[i] == column_name_) {
            column_index = i;
        }
    }
    if (column_index == columns_names.size()) {
        throw std::runtime_error(
            "Column not found: " + column_name_ + " :: TruncateToMinuteExpression");
    }
    size_t rows = batch->GetRowsNumber();
    std::shared_ptr<Column> result = std::make_shared<ColumnTyped<Timestamp>>(rows);
    for (size_t i = 0; i < rows; i++) {
        Timestamp ts = FromString<Timestamp>(batch->GetValue(i, column_index));
        Timestamp truncated{(ts.seconds / 60) * 60};
        result->SetValue(i, ToString<Timestamp>(truncated));
    }
    return result;
}

ExtractMinuteExpression::ExtractMinuteExpression(std::string column_name) {
    column_name_ = std::move(column_name);
}

std::shared_ptr<Column> ExtractMinuteExpression::Eval(std::shared_ptr<Batch> batch) const {
    std::vector<std::string> columns_names = batch->GetNames();
    size_t column_index = columns_names.size();
    for (size_t i = 0; i < columns_names.size(); i++) {
        if (columns_names[i] == column_name_) {
            column_index = i;
        }
    }
    if (column_index == columns_names.size()) {
        throw std::runtime_error(
            "Column not found: " + column_name_ + " :: ExtractMinuteExpression");
    }
    size_t rows = batch->GetRowsNumber();
    std::shared_ptr<Column> result = std::make_shared<ColumnTyped<int64_t>>(rows);
    for (size_t i = 0; i < rows; i++) {
        Timestamp ts = FromString<Timestamp>(batch->GetValue(i, column_index));
        int64_t minute = (ts.seconds / 60) % 60;
        result->SetValue(i, ToString<int64_t>(minute));
    }
    return result;
}
