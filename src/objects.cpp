#include "src/objects.h"

void SetBatchSize(size_t column_batch_size, size_t row_batch_size) {
    kColumnBatchSize = column_batch_size;
    kRowBatchSize = row_batch_size;
}

Date::Date() {
    days = 0;
}

Date::Date(int32_t d) {
    days = d;
}

Timestamp::Timestamp() {
    seconds = 0;
}

Timestamp::Timestamp(int64_t s) {
    seconds = s;
}

Batch::Batch(size_t rows_number, size_t columns_number, std::vector<Type> types, std::vector<std::string> names) {
    if (types.size() != columns_number || names.size() != columns_number) {
        throw std::runtime_error("columns_number mismatch :: Batch");
    }
    rows_number_ = rows_number;
    names_ = names;
    types_ = types;
    for (size_t i = 0; i < types.size(); i++) {
        switch (types[i]) {
            case Type::Int16:
                columns_.push_back(std::make_unique<ColumnTyped<int16_t>>(rows_number));
                break;
            case Type::Int32:
                columns_.push_back(std::make_unique<ColumnTyped<int32_t>>(rows_number));
                break;
            case Type::Int64:
                columns_.push_back(std::make_unique<ColumnTyped<int64_t>>(rows_number));
                break;
            case Type::Int128:
                // TODO ??????????????????????????????????????????????????????????????????????????????????????????
                break;
            case Type::Float:
                columns_.push_back(std::make_unique<ColumnTyped<float>>(rows_number));
                break;
            case Type::Double:
                columns_.push_back(std::make_unique<ColumnTyped<double>>(rows_number));
                break;
            case Type::Date:
                columns_.push_back(std::make_unique<ColumnTyped<Date>>(rows_number));
                break;
            case Type::Timestamp:
                columns_.push_back(std::make_unique<ColumnTyped<Timestamp>>(rows_number));
                break;
            case Type::Char:
                columns_.push_back(std::make_unique<ColumnTyped<char>>(rows_number));
                break;
            case Type::String:
                columns_.push_back(std::make_unique<ColumnTyped<std::string>>(rows_number));
                break;
            default:
                throw std::runtime_error("Invalid type :: Batch");
        }
    }
}

size_t Batch::GetColumnsNumber() const {
    return columns_.size();
}

size_t Batch::GetRowsNumber() const {
    return rows_number_;
}

std::vector<Type> Batch::GetTypes() const {
    return types_;
}

std::vector<std::string> Batch::GetNames() const {
    return names_;
}

void Batch::SetValue(size_t row_index, size_t column_index, std::string value) {
    if (columns_[column_index] == nullptr) {
        throw std::runtime_error("Null column :: Batch::SetValue");
    }
    columns_[column_index]->SetValue(row_index, value);
}

std::string Batch::GetValue(size_t row_index, size_t column_index) const {
    return columns_[column_index]->GetValue(row_index);
}
