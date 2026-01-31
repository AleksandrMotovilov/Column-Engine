#pragma once

#include <cstddef>
#include <variant>
#include <vector>
#include <string>
#include <memory>
#include <cstdint>

using Value = std::variant<std::string, int64_t>;

enum Type {
    INT64,
    STRING
};

class SchemaColumn {
public:
    SchemaColumn(std::string name, std::string type);
    SchemaColumn(std::string name, Type type);
    std::string GetString();
    std::string GetName();
    Type GetType();

private:
    std::string name_;
    Type type_;
};

class Schema {
public:
    Schema();
    void FromCSV(const std::string& file);
    void FromCLMN(const std::string& file);
    void Push(std::string name, Type type);
    size_t ColumnsNumber();
    std::string GetString();
    std::string GetName(size_t ind);
    Type GetType(size_t ind);

private:
    std::vector<SchemaColumn> columns_schema_;
};

class Column {
public:
    Column();
    void Push(std::string value, Type type);
    std::string GetValueString(size_t row);
    size_t RowsNumber();

private:
    std::vector<Value> column_;
};

class Table {
public:
    Table(std::shared_ptr<Schema> schema);
    void Push(size_t col, std::string value, Type type);
    std::string GetValueString(size_t row, size_t col);
    size_t RowsNumber();
    size_t ColumnsNumber();

private:
    std::vector<Column> columns_data_;
    std::shared_ptr<Schema> schema_;
};