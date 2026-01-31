#include "objects.h"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <string>

//----------------------SchemaColumn----------------------
SchemaColumn::SchemaColumn(std::string name, std::string type) {
    name_ = name;
    if (type == "int64") {
        type_ = Type::INT64;
    } else if (type == "string") {
        type_ = Type::STRING;
    } else {
        throw std::runtime_error("Invalid type: SchemaColumn::SchemaColumn");
    }
}
SchemaColumn::SchemaColumn(std::string name, Type type) {
    name_ = name;
    type_ = type;
}
std::string SchemaColumn::GetString() {
    switch (type_) {
        case Type::INT64:
            return name_ + ",int64";
        case Type::STRING:
            return name_ + ",string";
        default:
            throw std::runtime_error("Invalid type: SchemaColumn::GetString");
    }
}
std::string SchemaColumn::GetName() {
    return name_;
}
Type SchemaColumn::GetType() {
    return type_;
}

//----------------------Schema----------------------
Schema::Schema() {
    columns_schema_ = std::vector<SchemaColumn>();
}
void Schema::FromCSV(const std::string& file) {
    std::ifstream fin(file);
    if (!fin.is_open()) {
        throw std::runtime_error("Cannot open file {" + file + "}: Schema::FromCSV");
    }
    std::string line;
    while (std::getline(fin, line, '\n')) {
        std::stringstream ss;
        ss << line;
        std::string name;
        std::getline(ss, name, ',');
        std::string type;
        std::getline(ss, type, ',');
        std::string remaining;
        if (std::getline(ss, remaining, '\n')) {
            if (remaining != "") {
                throw std::runtime_error("Invalid schema format at line {ends on " + remaining + "}: Schema::FromCSV");
            }
        }
        columns_schema_.emplace_back(name, type);
    }
    fin.close();
}
void Schema::FromCLMN(const std::string& file) {
    std::ifstream fin(file);
    if (!fin.is_open()) {
        throw std::runtime_error("Cannot open file {" + file + "}: Schema::FromCLMN");
    }
    int64_t offset;
    fin.seekg(-sizeof(offset), std::ios::end);
    fin.read(reinterpret_cast<char*>(&offset), sizeof(offset));
    fin.seekg(0, std::ios::end);
    std::streampos fin_end = fin.tellg();
    std::streampos schema_end = fin_end - static_cast<std::streampos>(sizeof(offset));
    fin.seekg(-offset, std::ios::end);
    std::string line;
    std::streampos pos = fin.tellg();
    while (pos < schema_end && std::getline(fin, line, '\n')) {
        pos = fin.tellg();
        std::stringstream ss;
        ss << line;
        std::string name;
        std::getline(ss, name, ',');
        std::string type;
        std::getline(ss, type, ',');
        std::string remaining;
        if (std::getline(ss, remaining, '\n')) {
            if (remaining != "") {
                throw std::runtime_error("Invalid schema format at line: {ends on " + remaining + "}: Schema::FromCLMN");
            }
        }
        columns_schema_.emplace_back(name, type);
    }
    fin.close();
}
void Schema::Push(std::string name, Type type) {
    columns_schema_.emplace_back(name, type);
}
size_t Schema::ColumnsNumber() {
    return columns_schema_.size();
}
std::string Schema::GetString() {
    std::string s;
    for (size_t i = 0; i < columns_schema_.size(); i++) {
        s = s + columns_schema_[i].GetString() + '\n';
    }
    return s;
}
std::string Schema::GetName(size_t col) {
    if (col >= columns_schema_.size()) {
        throw std::runtime_error("Index out of range {index " + std::to_string(col) + ", size " + std::to_string(columns_schema_.size()) + "}: Schema::GetName");
    }
    return columns_schema_[col].GetName();
}
Type Schema::GetType(size_t col) {
    if (col >= columns_schema_.size()) {
        throw std::runtime_error("Index out of range {index " + std::to_string(col) + ", size " + std::to_string(columns_schema_.size()) + "}: Schema::GetType");
    }
    return columns_schema_[col].GetType();
}

//----------------------Column----------------------
Column::Column() {
    column_ = std::vector<Value>(0);
}
void Column::Push(std::string value, Type type) {
    switch (type) {
        case Type::INT64:
            try{
                column_.push_back(std::stoll(value));
            } catch(...) {
                throw std::runtime_error("Invalid type: Column::Push Type::INT64");
            }
            break;
        case Type::STRING:
            column_.push_back(value);
            break;
        default:
            throw std::runtime_error("Invalid type: Column::Push");
    }
}
std::string Column::GetValueString(size_t row) {
    if (row >= column_.size()) {
        throw std::runtime_error("Index out of range {index " + std::to_string(row) + ", size " + std::to_string(column_.size()) + "}: Column::GetValueString");
    }
    if (std::holds_alternative<int64_t>(column_[row])) {
        return std::to_string(std::get<int64_t>(column_[row]));
    }
    if (std::holds_alternative<std::string>(column_[row])) {
        return std::get<std::string>(column_[row]);
    } 
    throw std::runtime_error("Invalid type: Column::GetValueString");
}
size_t Column::RowsNumber() {
    return column_.size();
}

//----------------------Table----------------------
Table::Table(std::shared_ptr<Schema> schema) {
    schema_ = schema;
    for (size_t i = 0; i < schema_->ColumnsNumber(); i++) {
        columns_data_.emplace_back();
    }
}
void Table::Push(size_t col, std::string value, Type type) {
    if (col >= columns_data_.size()) {
        throw std::runtime_error("Index out of range {index " + std::to_string(col) + ", size " + std::to_string(columns_data_.size()) + "}: Table::Push");
    }
    columns_data_[col].Push(value, type);
}
std::string Table::GetValueString(size_t row, size_t col) {
    if (col >= columns_data_.size()) {
        throw std::runtime_error("Index out of range {index " + std::to_string(col) + ", size " + std::to_string(columns_data_.size()) + "}: Table::GetValueString");
    }
    return columns_data_[col].GetValueString(row);
}
size_t Table::RowsNumber() {
    if (columns_data_.size() == 0) {
        return 0;
    }
    return columns_data_[0].RowsNumber();
}
size_t Table::ColumnsNumber() {
    return columns_data_.size();
}