#include "objects.h"

Type TypeFromString(std::string value) {
    if (value == "int16") {
        return Type::Int16;
    } else if (value == "int32") {
        return Type::Int32;
    } else if (value == "int64") {
        return Type::Int64;
    } else if (value == "int128") {
        return Type::Int128;
    } else if (value == "float") {
        return Type::Float;
    } else if (value == "double") {
        return Type::Double;
    } else if (value == "date") {
        return Type::Date;
    } else if (value == "timestamp") {
        return Type::Timestamp;
    } else if (value == "char") {
        return Type::Char;
    } else if (value == "string") {
        return Type::String;
    } else {
        throw std::runtime_error("Invalid type :: TypeFromString");
    }
}
std::string StringFromType(Type value) {
    if (value == Type::Int16) {
        return "int16";
    } else if (value == Type::Int32) {
        return "int32";
    } else if (value == Type::Int64) {
        return "int64";
    } else if (value == Type::Int128) {
        return "int128";
    } else if (value == Type::Float) {
        return "float";
    } else if (value == Type::Double) {
        return "double";
    } else if (value == Type::Date) {
        return "date";
    } else if (value == Type::Timestamp) {
        return "timestamp";
    } else if (value == Type::Char) {
        return "char";
    } else if (value == Type::String) {
        return "string";
    } else {
        throw std::runtime_error("Invalid type :: StringFromType");
    }
}

Batch::Batch(size_t rows_number, std::vector<Type> types, std::vector<std::string> names) {
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
                // TODO ??????????????????????????????????????????????????????????????????????????????????????????
                break;
            case Type::Timestamp:
                // TODO ??????????????????????????????????????????????????????????????????????????????????????????
                break;
            case Type::Char:
                columns_.push_back(std::make_unique<ColumnTyped<char>>(rows_number));
                break;
            case Type::String:
                columns_.push_back(std::make_unique<ColumnTyped<std::string>>(rows_number));
                break;
            default:
                throw std::runtime_error("Invalid type");
        }
    }
}
size_t Batch::GetSize() {
    return columns_.size();
}
std::vector<Type> Batch::GetTypes() {
    return types_;
}
std::vector<std::string> Batch::GetNames() {
    return names_;
}
void Batch::SetValue(size_t row_index, size_t column_index, std::string value) {
    if (columns_[column_index] == nullptr) {
        throw std::runtime_error("Error");
    }
    columns_[column_index]->SetValue(row_index, value);
}
std::string Batch::GetValue(size_t index_column, size_t index_row) {
    return columns_[index_column]->GetValue(index_row);
}

Table::Table(const std::string& file_input, const std::string& file_output) {
    if (file_input == file_output) {
        throw std::runtime_error("Error");
    }
    std::ifstream fin(file_input);
    fin_ = std::move(fin);
    if (!fin_.is_open()) {
        throw std::runtime_error("Cannot open file {" + file_input + "}");
    }
    std::ofstream fout(file_output);
    fout_ = std::move(fout);
    if (!fout_.is_open()) {
        throw std::runtime_error("Cannot open file {" + file_output + "}");
    }
    fin_.read(reinterpret_cast<char*>(&rows_number_), sizeof(size_t));
    fin_.read(reinterpret_cast<char*>(&columns_number_), sizeof(size_t));
    // Точно ли здесь так и если нент, то как надо?
    fout_.write(reinterpret_cast<char*>(&rows_number_), sizeof(size_t));
    fout_.write(reinterpret_cast<char*>(&columns_number_), sizeof(size_t));
    column_batch_index_ = 0;
    ReadBatch();
}
Table::~Table() {
    fin_.close();
    fout_.close();
}
void Table::ReadBatch() {
    size_t column_batch_index_check;
    size_t column_batch_size;
    fin_.read(reinterpret_cast<char*>(&column_batch_index_check), sizeof(size_t));
    fin_.read(reinterpret_cast<char*>(&column_batch_size), sizeof(size_t));

    if (column_batch_index_ != column_batch_index_check) {
        throw std::runtime_error("Error");
    }

    std::vector<std::vector<std::string>> columns(column_batch_size, std::vector<std::string>(rows_number_));
    std::vector<Type> types(column_batch_size);
    std::vector<std::string> names(column_batch_size);
    for (size_t i = 0; i < column_batch_size; i++) {
        fin_.read(reinterpret_cast<char*>(&types[i]), sizeof(uint8_t));
        size_t size_str;
        fin_.read(reinterpret_cast<char*>(&size_str), sizeof(size_t));
        std::string str(size_str, '\0');
        fin_.read(&str[0], size_str * sizeof(char));
        names[i] = str;
    }

    batch_ = std::make_unique<Batch>(rows_number_, types, names);

    for (size_t i = 0; i < column_batch_size; i++) {
        if (types[i] == Type::Int16) {
            std::vector<int16_t> column(rows_number_);
            fin_.read(reinterpret_cast<char*>(&column[0]), rows_number_ * sizeof(int16_t));
            for (size_t j = 0; j < rows_number_; j++) {
                batch_->SetValue(j, i, ToString<int16_t>(column[j]));
            }
        } else if (types[i] == Type::Int32) {
            std::vector<int32_t> column(rows_number_);
            fin_.read(reinterpret_cast<char*>(&column[0]), rows_number_ * sizeof(int32_t));
            for (size_t j = 0; j < rows_number_; j++) {
                batch_->SetValue(j, i, ToString<int32_t>(column[j]));
            }
        } else if (types[i] == Type::Int64) {
            std::vector<int64_t> column(rows_number_);
            fin_.read(reinterpret_cast<char*>(&column[0]), rows_number_ * sizeof(int64_t));
            for (size_t j = 0; j < rows_number_; j++) {
                batch_->SetValue(j, i, ToString<int64_t>(column[j]));
            }
        } else if (types[i] == Type::Int128) {
            // TODO ??????????????????????????????????????????????????????????????????????????????????????????
        } else if (types[i] == Type::Float) {
            std::vector<float> column(rows_number_);
            fin_.read(reinterpret_cast<char*>(&column[0]), rows_number_ * sizeof(float));
            for (size_t j = 0; j < rows_number_; j++) {
                batch_->SetValue(j, i, ToString<float>(column[j]));
            }
        } else if (types[i] == Type::Double) {
            std::vector<double> column(rows_number_);
            fin_.read(reinterpret_cast<char*>(&column[0]), rows_number_ * sizeof(double));
            for (size_t j = 0; j < rows_number_; j++) {
                batch_->SetValue(j, i, ToString<double>(column[j]));
            }
        } else if (types[i] == Type::Date) {
            // TODO ??????????????????????????????????????????????????????????????????????????????????????????
        } else if (types[i] == Type::Timestamp) {
            // TODO ??????????????????????????????????????????????????????????????????????????????????????????
        } else if (types[i] == Type::Char) {
            std::vector<char> column(rows_number_);
            fin_.read(reinterpret_cast<char*>(&column[0]), rows_number_ * sizeof(char));
            for (size_t j = 0; j < rows_number_; j++) {
                batch_->SetValue(j, i, ToString<char>(column[j]));
            }
        } else if (types[i] == Type::String) {
            size_t size;
            for (size_t j = 0; j < rows_number_; j++) {
                fin_.read(reinterpret_cast<char*>(&size), sizeof(size_t));
                std::string value(size, '\0');
                fin_.read(&value[0], size * sizeof(char));
                batch_->SetValue(j, i, ToString<std::string>(value));
            }
        } else {
            throw std::runtime_error("Invalid type");
        }
    }

    column_batch_index_ = column_batch_index_ + column_batch_size;
}
void Table::WriteBatch() {
    size_t column_batch_size = batch_->GetSize();
    std::vector<Type> types = batch_->GetTypes();
    std::vector<std::string> names = batch_->GetNames();
    fout_.write(reinterpret_cast<char*>(&column_batch_index_), sizeof(size_t));
    fout_.write(reinterpret_cast<char*>(&column_batch_size), sizeof(size_t));
    for (size_t i = 0; i < column_batch_size; i++) {
        fout_.write(reinterpret_cast<char*>(&types[i]), sizeof(uint8_t));
        size_t size_str = names[i].size();
        fout_.write(reinterpret_cast<char*>(&size_str), sizeof(size_t));
        fout_.write(names[i].c_str(), size_str * sizeof(char));
    }

    for (size_t i = 0; i < column_batch_size; i++) {
        if (types[i] == Type::Int16) {
            std::vector<int16_t> value(rows_number_);
            for (size_t j = 0; j < rows_number_; j++) {
                value[j] = FromString<int16_t>(batch_->GetValue(j, i));
            }
            fout_.write(reinterpret_cast<char*>(&value[0]), rows_number_ * sizeof(int16_t));
        } else if (types[i] == Type::Int32) {
            std::vector<int32_t> value(rows_number_);
            for (size_t j = 0; j < rows_number_; j++) {
                value[j] = FromString<int32_t>(batch_->GetValue(j, i));
            }
            fout_.write(reinterpret_cast<char*>(&value[0]), rows_number_ * sizeof(int32_t));
        } else if (types[i] == Type::Int64) {
            std::vector<int64_t> value(rows_number_);
            for (size_t j = 0; j < rows_number_; j++) {
                value[j] = FromString<int64_t>(batch_->GetValue(j, i));
            }
            fout_.write(reinterpret_cast<char*>(&value[0]), rows_number_ * sizeof(int64_t));
        } else if (types[i] == Type::Int128) {
            // TODO ??????????????????????????????????????????????????????????????????????????????????????????
        } else if (types[i] == Type::Float) {
            std::vector<float> value(rows_number_);
            for (size_t j = 0; j < rows_number_; j++) {
                value[j] = FromString<float>(batch_->GetValue(j, i));
            }
            fout_.write(reinterpret_cast<char*>(&value[0]), rows_number_ * sizeof(float));
        } else if (types[i] == Type::Double) {
            std::vector<double> value(rows_number_);
            for (size_t j = 0; j < rows_number_; j++) {
                value[j] = FromString<double>(batch_->GetValue(j, i));
            }
            fout_.write(reinterpret_cast<char*>(&value[0]), rows_number_ * sizeof(double));
        } else if (types[i] == Type::Date) {
            // TODO ??????????????????????????????????????????????????????????????????????????????????????????
        } else if (types[i] == Type::Timestamp) {
            // TODO ??????????????????????????????????????????????????????????????????????????????????????????
        } else if (types[i] == Type::Char) {
            std::vector<char> value(rows_number_);
            for (size_t j = 0; j < rows_number_; j++) {
                value[j] = FromString<char>(batch_->GetValue(j, i));
            }
            fout_.write(reinterpret_cast<char*>(&value[0]), rows_number_ * sizeof(char));
        } else if (types[i] == Type::String) {
            size_t size;
            for (size_t j = 0; j < rows_number_; j++) {
                std::string value = FromString<std::string>(batch_->GetValue(j, i));
                size = value.size();
                fout_.write(reinterpret_cast<char*>(&size), sizeof(size_t));
                fout_.write(value.c_str(), size * sizeof(char));
            }
        } else {
            throw std::runtime_error("Invalid type");
        }
    }
}