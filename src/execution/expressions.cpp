#include "src/execution/expressions.h"

EqualExpression::EqualExpression(std::string column_name, std::string value) {
    column_name_ = std::move(column_name);
    value_ = std::move(value);
}

std::shared_ptr<Column> EqualExpression::Eval(std::shared_ptr<Batch> batch) const {
    size_t index = batch->FindColumn(column_name_);
    if (index == batch->GetColumnsNumber()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: EqualExpression");
    }
    Type column_type = batch->GetType(index);
    size_t rows_number = batch->GetRowsNumber();
    std::vector<char> result(rows_number);
    switch (column_type) {
        case Type::Int16: {
            const std::vector<int16_t>& column_data = static_cast<const ColumnTyped<int16_t>&>(*batch->GetColumn(index)).GetData();
            int16_t threshold = FromString<int16_t>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] == threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Int32: {
            const std::vector<int32_t>& column_data = static_cast<const ColumnTyped<int32_t>&>(*batch->GetColumn(index)).GetData();
            int32_t threshold = FromString<int32_t>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] == threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Int64: {
            const std::vector<int64_t>& column_data = static_cast<const ColumnTyped<int64_t>&>(*batch->GetColumn(index)).GetData();
            int64_t threshold = FromString<int64_t>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] == threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Float: {
            const std::vector<float>& column_data = static_cast<const ColumnTyped<float>&>(*batch->GetColumn(index)).GetData();
            float threshold = FromString<float>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] == threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Double: {
            const std::vector<double>& column_data = static_cast<const ColumnTyped<double>&>(*batch->GetColumn(index)).GetData();
            double threshold = FromString<double>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] == threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Date: {
            const std::vector<Date>& column_data = static_cast<const ColumnTyped<Date>&>(*batch->GetColumn(index)).GetData();
            Date threshold = FromString<Date>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] == threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Timestamp: {
            const std::vector<Timestamp>& column_data = static_cast<const ColumnTyped<Timestamp>&>(*batch->GetColumn(index)).GetData();
            Timestamp threshold = FromString<Timestamp>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] == threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Char: {
            const std::vector<char>& column_data = static_cast<const ColumnTyped<char>&>(*batch->GetColumn(index)).GetData();
            char threshold = FromString<char>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] == threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::String: {
            const std::vector<std::string>& column_data = static_cast<const ColumnTyped<std::string>&>(*batch->GetColumn(index)).GetData();
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] == value_) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        default: {
            throw std::runtime_error("Unsupported type :: EqualExpression");
        }
    }
    return std::make_shared<ColumnTyped<char>>(std::move(result));
}

Type EqualExpression::GetType() const {
    return Type::Char;
}

NotEqualExpression::NotEqualExpression(std::string column_name, std::string value) {
    column_name_ = std::move(column_name);
    value_ = std::move(value);
}

std::shared_ptr<Column> NotEqualExpression::Eval(std::shared_ptr<Batch> batch) const {
    size_t index = batch->FindColumn(column_name_);
    if (index == batch->GetColumnsNumber()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: NotEqualExpression");
    }
    Type column_type = batch->GetType(index);
    size_t rows_number = batch->GetRowsNumber();
    std::vector<char> result(rows_number);
    switch (column_type) {
        case Type::Int16: {
            const std::vector<int16_t>& column_data = static_cast<const ColumnTyped<int16_t>&>(*batch->GetColumn(index)).GetData();
            int16_t threshold = FromString<int16_t>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] != threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Int32: {
            const std::vector<int32_t>& column_data = static_cast<const ColumnTyped<int32_t>&>(*batch->GetColumn(index)).GetData();
            int32_t threshold = FromString<int32_t>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] != threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Int64: {
            const std::vector<int64_t>& column_data = static_cast<const ColumnTyped<int64_t>&>(*batch->GetColumn(index)).GetData();
            int64_t threshold = FromString<int64_t>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] != threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Float: {
            const std::vector<float>& column_data = static_cast<const ColumnTyped<float>&>(*batch->GetColumn(index)).GetData();
            float threshold = FromString<float>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] != threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Double: {
            const std::vector<double>& column_data = static_cast<const ColumnTyped<double>&>(*batch->GetColumn(index)).GetData();
            double threshold = FromString<double>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] != threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Date: {
            const std::vector<Date>& column_data = static_cast<const ColumnTyped<Date>&>(*batch->GetColumn(index)).GetData();
            Date threshold = FromString<Date>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] != threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Timestamp: {
            const std::vector<Timestamp>& column_data = static_cast<const ColumnTyped<Timestamp>&>(*batch->GetColumn(index)).GetData();
            Timestamp threshold = FromString<Timestamp>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] != threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Char: {
            const std::vector<char>& column_data = static_cast<const ColumnTyped<char>&>(*batch->GetColumn(index)).GetData();
            char threshold = FromString<char>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] != threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::String: {
            const std::vector<std::string>& column_data = static_cast<const ColumnTyped<std::string>&>(*batch->GetColumn(index)).GetData();
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] != value_) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        default: {
            throw std::runtime_error("Unsupported type :: NotEqualExpression");
        }
    }
    return std::make_shared<ColumnTyped<char>>(std::move(result));
}

Type NotEqualExpression::GetType() const {
    return Type::Char;
}

GreaterOrEqualExpression::GreaterOrEqualExpression(std::string column_name, std::string value) {
    column_name_ = std::move(column_name);
    value_ = std::move(value);
}

std::shared_ptr<Column> GreaterOrEqualExpression::Eval(std::shared_ptr<Batch> batch) const {
    size_t index = batch->FindColumn(column_name_);
    if (index == batch->GetColumnsNumber()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: GreaterOrEqualExpression");
    }
    Type column_type = batch->GetType(index);
    size_t rows_number = batch->GetRowsNumber();
    std::vector<char> result(rows_number);
    switch (column_type) {
        case Type::Int16: {
            const std::vector<int16_t>& column_data = static_cast<const ColumnTyped<int16_t>&>(*batch->GetColumn(index)).GetData();
            int16_t threshold = FromString<int16_t>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] >= threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Int32: {
            const std::vector<int32_t>& column_data = static_cast<const ColumnTyped<int32_t>&>(*batch->GetColumn(index)).GetData();
            int32_t threshold = FromString<int32_t>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] >= threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Int64: {
            const std::vector<int64_t>& column_data = static_cast<const ColumnTyped<int64_t>&>(*batch->GetColumn(index)).GetData();
            int64_t threshold = FromString<int64_t>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] >= threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Float: {
            const std::vector<float>& column_data = static_cast<const ColumnTyped<float>&>(*batch->GetColumn(index)).GetData();
            float threshold = FromString<float>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] >= threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Double: {
            const std::vector<double>& column_data = static_cast<const ColumnTyped<double>&>(*batch->GetColumn(index)).GetData();
            double threshold = FromString<double>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] >= threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Date: {
            const std::vector<Date>& column_data = static_cast<const ColumnTyped<Date>&>(*batch->GetColumn(index)).GetData();
            Date threshold = FromString<Date>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] >= threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Timestamp: {
            const std::vector<Timestamp>& column_data = static_cast<const ColumnTyped<Timestamp>&>(*batch->GetColumn(index)).GetData();
            Timestamp threshold = FromString<Timestamp>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] >= threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::String: {
            const std::vector<std::string>& column_data = static_cast<const ColumnTyped<std::string>&>(*batch->GetColumn(index)).GetData();
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] >= value_) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        default: {
            throw std::runtime_error("Unsupported type :: GreaterOrEqualExpression");
        }
    }
    return std::make_shared<ColumnTyped<char>>(std::move(result));
}

Type GreaterOrEqualExpression::GetType() const {
    return Type::Char;
}

LessOrEqualExpression::LessOrEqualExpression(std::string column_name, std::string value) {
    column_name_ = std::move(column_name);
    value_ = std::move(value);
}

std::shared_ptr<Column> LessOrEqualExpression::Eval(std::shared_ptr<Batch> batch) const {
    size_t index = batch->FindColumn(column_name_);
    if (index == batch->GetColumnsNumber()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: LessOrEqualExpression");
    }
    Type column_type = batch->GetType(index);
    size_t rows_number = batch->GetRowsNumber();
    std::vector<char> result(rows_number);
    switch (column_type) {
        case Type::Int16: {
            const std::vector<int16_t>& column_data = static_cast<const ColumnTyped<int16_t>&>(*batch->GetColumn(index)).GetData();
            int16_t threshold = FromString<int16_t>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] <= threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Int32: {
            const std::vector<int32_t>& column_data = static_cast<const ColumnTyped<int32_t>&>(*batch->GetColumn(index)).GetData();
            int32_t threshold = FromString<int32_t>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] <= threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Int64: {
            const std::vector<int64_t>& column_data = static_cast<const ColumnTyped<int64_t>&>(*batch->GetColumn(index)).GetData();
            int64_t threshold = FromString<int64_t>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] <= threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Float: {
            const std::vector<float>& column_data = static_cast<const ColumnTyped<float>&>(*batch->GetColumn(index)).GetData();
            float threshold = FromString<float>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] <= threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Double: {
            const std::vector<double>& column_data = static_cast<const ColumnTyped<double>&>(*batch->GetColumn(index)).GetData();
            double threshold = FromString<double>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] <= threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Date: {
            const std::vector<Date>& column_data = static_cast<const ColumnTyped<Date>&>(*batch->GetColumn(index)).GetData();
            Date threshold = FromString<Date>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] <= threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::Timestamp: {
            const std::vector<Timestamp>& column_data = static_cast<const ColumnTyped<Timestamp>&>(*batch->GetColumn(index)).GetData();
            Timestamp threshold = FromString<Timestamp>(value_);
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] <= threshold) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        case Type::String: {
            const std::vector<std::string>& column_data = static_cast<const ColumnTyped<std::string>&>(*batch->GetColumn(index)).GetData();
            for (size_t i = 0; i < rows_number; i++) {
                if (column_data[i] <= value_) {
                    result[i] = 1;
                } else {
                    result[i] = 0;
                }
            }
            break;
        }
        default: {
            throw std::runtime_error("Unsupported type :: LessOrEqualExpression");
        }
    }
    return std::make_shared<ColumnTyped<char>>(std::move(result));
}

Type LessOrEqualExpression::GetType() const {
    return Type::Char;
}

ContainsExpression::ContainsExpression(std::string column_name, std::string substring) {
    column_name_ = std::move(column_name);
    substring_ = std::move(substring);
}

std::shared_ptr<Column> ContainsExpression::Eval(std::shared_ptr<Batch> batch) const {
    size_t index = batch->FindColumn(column_name_);
    if (index == batch->GetColumnsNumber()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: ContainsExpression");
    }
    const std::vector<std::string>& column_data = static_cast<const ColumnTyped<std::string>&>(*batch->GetColumn(index)).GetData();
    size_t rows_number = batch->GetRowsNumber();
    std::vector<char> result(rows_number);
    for (size_t i = 0; i < rows_number; i++) {
        if (column_data[i].find(substring_) != std::string::npos) {
            result[i] = 1;
        } else {
            result[i] = 0;
        }
    }
    return std::make_shared<ColumnTyped<char>>(std::move(result));
}

Type ContainsExpression::GetType() const {
    return Type::Char;
}

NotContainsExpression::NotContainsExpression(std::string column_name, std::string substring) {
    column_name_ = std::move(column_name);
    substring_ = std::move(substring);
}

std::shared_ptr<Column> NotContainsExpression::Eval(std::shared_ptr<Batch> batch) const {
    size_t index = batch->FindColumn(column_name_);
    if (index == batch->GetColumnsNumber()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: NotContainsExpression");
    }
    const std::vector<std::string>& column_data = static_cast<const ColumnTyped<std::string>&>(*batch->GetColumn(index)).GetData();
    size_t rows_number = batch->GetRowsNumber();
    std::vector<char> result(rows_number);
    for (size_t i = 0; i < rows_number; i++) {
        if (column_data[i].find(substring_) == std::string::npos) {
            result[i] = 1;
        } else {
            result[i] = 0;
        }
    }
    return std::make_shared<ColumnTyped<char>>(std::move(result));
}

Type NotContainsExpression::GetType() const {
    return Type::Char;
}

AndExpression::AndExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right) {
    left_ = std::move(left);
    right_ = std::move(right);
}

std::shared_ptr<Column> AndExpression::Eval(std::shared_ptr<Batch> batch) const {
    std::shared_ptr<Column> left_col = left_->Eval(batch);
    std::shared_ptr<Column> right_col = right_->Eval(batch);
    const std::vector<char>& left_data = static_cast<const ColumnTyped<char>&>(*left_col).GetData();
    const std::vector<char>& right_data = static_cast<const ColumnTyped<char>&>(*right_col).GetData();
    size_t rows_number = batch->GetRowsNumber();
    std::vector<char> result(rows_number);
    for (size_t i = 0; i < rows_number; i++) {
        if (left_data[i] != 0 && right_data[i] != 0) {
            result[i] = 1;
        } else {
            result[i] = 0;
        }
    }
    return std::make_shared<ColumnTyped<char>>(std::move(result));
}

Type AndExpression::GetType() const {
    return Type::Char;
}

OrExpression::OrExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right) {
    left_ = std::move(left);
    right_ = std::move(right);
}

std::shared_ptr<Column> OrExpression::Eval(std::shared_ptr<Batch> batch) const {
    std::shared_ptr<Column> left_column = left_->Eval(batch);
    std::shared_ptr<Column> right_column = right_->Eval(batch);
    const std::vector<char>& left_data = static_cast<const ColumnTyped<char>&>(*left_column).GetData();
    const std::vector<char>& right_data = static_cast<const ColumnTyped<char>&>(*right_column).GetData();
    size_t rows_number = batch->GetRowsNumber();
    std::vector<char> result(rows_number);
    for (size_t i = 0; i < rows_number; i++) {
        if (left_data[i] != 0 || right_data[i] != 0) {
            result[i] = 1;
        } else {
            result[i] = 0;
        }
    }
    return std::make_shared<ColumnTyped<char>>(std::move(result));
}

Type OrExpression::GetType() const {
    return Type::Char;
}

ConstantExpression::ConstantExpression(std::string value) {
    value_ = std::move(value);
}

std::shared_ptr<Column> ConstantExpression::Eval(std::shared_ptr<Batch> batch) const {
    size_t rows_number = batch->GetRowsNumber();
    std::vector<std::string> result(rows_number, value_);
    return std::make_shared<ColumnTyped<std::string>>(std::move(result));
}

Type ConstantExpression::GetType() const {
    return Type::String;
}

SumExpression::SumExpression(std::string column_name, int64_t constant) {
    column_name_ = std::move(column_name);
    constant_ = constant;
}

std::shared_ptr<Column> SumExpression::Eval(std::shared_ptr<Batch> batch) const {
    size_t index = batch->FindColumn(column_name_);
    if (index == batch->GetColumnsNumber()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: SumExpression");
    }
    Type column_type = batch->GetType(index);
    size_t rows_number = batch->GetRowsNumber();
    std::vector<int64_t> result(rows_number);
    switch (column_type) {
        case Type::Int16: {
            const std::vector<int16_t>& column_data = static_cast<const ColumnTyped<int16_t>&>(*batch->GetColumn(index)).GetData();
            for (size_t i = 0; i < rows_number; i++) {
                result[i] = static_cast<int64_t>(column_data[i]) + constant_;
            }
            break;
        }
        case Type::Int32: {
            const std::vector<int32_t>& column_data = static_cast<const ColumnTyped<int32_t>&>(*batch->GetColumn(index)).GetData();
            for (size_t i = 0; i < rows_number; i++) {
                result[i] = static_cast<int64_t>(column_data[i]) + constant_;
            }
            break;
        }
        case Type::Int64: {
            const std::vector<int64_t>& column_data = static_cast<const ColumnTyped<int64_t>&>(*batch->GetColumn(index)).GetData();
            for (size_t i = 0; i < rows_number; i++) {
                result[i] = column_data[i] + constant_;
            }
            break;
        }
        default: {
            throw std::runtime_error("Unsupported type :: SumExpression");
        }
    }
    return std::make_shared<ColumnTyped<int64_t>>(std::move(result));
}

Type SumExpression::GetType() const {
    return Type::Int64;
}

LengthExpression::LengthExpression(std::string column_name) {
    column_name_ = std::move(column_name);
}

std::shared_ptr<Column> LengthExpression::Eval(std::shared_ptr<Batch> batch) const {
    size_t index = batch->FindColumn(column_name_);
    if (index == batch->GetColumnsNumber()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: LengthExpression");
    }
    const std::vector<std::string>& column_data = static_cast<const ColumnTyped<std::string>&>(*batch->GetColumn(index)).GetData();
    size_t rows_number = batch->GetRowsNumber();
    std::vector<int64_t> result(rows_number);
    for (size_t i = 0; i < rows_number; i++) {
        result[i] = static_cast<int64_t>(column_data[i].size());
    }
    return std::make_shared<ColumnTyped<int64_t>>(std::move(result));
}

Type LengthExpression::GetType() const {
    return Type::Int64;
}

RegexpReplaceExpression::RegexpReplaceExpression(std::string column_name, std::string pattern, std::string replacement) : re_(pattern) {
    column_name_ = std::move(column_name);
    replacement_ = std::move(replacement);
}

std::shared_ptr<Column> RegexpReplaceExpression::Eval(std::shared_ptr<Batch> batch) const {
    size_t index = batch->FindColumn(column_name_);
    if (index == batch->GetColumnsNumber()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: RegexpReplaceExpression");
    }
    const std::vector<std::string>& column_data = static_cast<const ColumnTyped<std::string>&>(*batch->GetColumn(index)).GetData();
    size_t rows_number = batch->GetRowsNumber();
    std::vector<std::string> result(rows_number);
    for (size_t i = 0; i < rows_number; i++) {
        result[i] = column_data[i];
        RE2::Replace(&result[i], re_, replacement_);
    }
    return std::make_shared<ColumnTyped<std::string>>(std::move(result));
}

Type RegexpReplaceExpression::GetType() const {
    return Type::String;
}

CaseWhenExpression::CaseWhenExpression(std::shared_ptr<Expression> condition, std::string then_column, std::string else_value) {
    condition_ = std::move(condition);
    then_column_ = std::move(then_column);
    else_value_ = std::move(else_value);
}

std::shared_ptr<Column> CaseWhenExpression::Eval(std::shared_ptr<Batch> batch) const {
    std::shared_ptr<Column> mask_column = condition_->Eval(batch);
    const std::vector<char>& mask = static_cast<const ColumnTyped<char>&>(*mask_column).GetData();
    size_t index = batch->FindColumn(then_column_);
    if (index == batch->GetColumnsNumber()) {
        throw std::runtime_error("Column not found: " + then_column_ + " :: CaseWhenExpression");
    }
    const std::vector<std::string>& column_data = static_cast<const ColumnTyped<std::string>&>(*batch->GetColumn(index)).GetData();
    size_t rows_number = batch->GetRowsNumber();
    std::vector<std::string> result(rows_number);
    for (size_t i = 0; i < rows_number; i++) {
        if (mask[i] != 0) {
            result[i] = column_data[i];
        } else {
            result[i] = else_value_;
        }
    }
    return std::make_shared<ColumnTyped<std::string>>(std::move(result));
}

Type CaseWhenExpression::GetType() const {
    return Type::String;
}

TruncateToMinuteExpression::TruncateToMinuteExpression(std::string column_name) {
    column_name_ = std::move(column_name);
}

std::shared_ptr<Column> TruncateToMinuteExpression::Eval(std::shared_ptr<Batch> batch) const {
    size_t index = batch->FindColumn(column_name_);
    if (index == batch->GetColumnsNumber()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: TruncateToMinuteExpression");
    }
    const std::vector<Timestamp>& column_data = static_cast<const ColumnTyped<Timestamp>&>(*batch->GetColumn(index)).GetData();
    size_t rows_number = batch->GetRowsNumber();
    std::vector<Timestamp> result(rows_number);
    for (size_t i = 0; i < rows_number; i++) {
        result[i] = Timestamp{(column_data[i].GetValue() / 60) * 60};
    }
    return std::make_shared<ColumnTyped<Timestamp>>(std::move(result));
}

Type TruncateToMinuteExpression::GetType() const {
    return Type::Timestamp;
}

ExtractMinuteExpression::ExtractMinuteExpression(std::string column_name) {
    column_name_ = std::move(column_name);
}

std::shared_ptr<Column> ExtractMinuteExpression::Eval(std::shared_ptr<Batch> batch) const {
    size_t index = batch->FindColumn(column_name_);
    if (index == batch->GetColumnsNumber()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: ExtractMinuteExpression");
    }
    const std::vector<Timestamp>& column_data = static_cast<const ColumnTyped<Timestamp>&>(*batch->GetColumn(index)).GetData();
    size_t rows_number = batch->GetRowsNumber();
    std::vector<int64_t> result(rows_number);
    for (size_t i = 0; i < rows_number; i++) {
        result[i] = (column_data[i].GetValue() / 60) % 60;
    }
    return std::make_shared<ColumnTyped<int64_t>>(std::move(result));
}

Type ExtractMinuteExpression::GetType() const {
    return Type::Int64;
}
