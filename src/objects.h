#pragma once

#include <cstddef>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <cstdint>

const size_t kColumnBatchSize = 2;
const size_t kRowBatchSize = 2;

enum Type : uint8_t {
    Int16,
    Int32,
    Int64,
    Int128,
    Float,
    Double,
    Date,
    Timestamp,
    Char,
    String,
};

Type TypeFromString(std::string value);
std::string StringFromType(Type value);

template<typename T>
T FromString(std::string str) {
    std::stringstream ss;
    ss << str;
    T value;
    ss >> value;
    return value;
}

template<typename T>
std::string ToString(T value) {
    std::stringstream ss;
    ss << value;
    std::string str;
    str = ss.str();
    return str;
}

class Column {
public:
    virtual ~Column() = default; 
    virtual size_t GetSize() = 0;
    ////// Можно ли сделать умнее, чем передавать через string? //////
    virtual void SetValue(size_t index, std::string value) = 0;
    virtual std::string GetValue(size_t index) = 0;
};

template<typename T>
class ColumnTyped : public Column {
public:
    ColumnTyped(size_t size) {
        column_ = std::vector<T>(size);
    }
    ~ColumnTyped() {
    }
    size_t GetSize() override {
        return column_.size();
    }
    void SetValue(size_t index, std::string value) override {
        column_[index] = FromString<T>(value);
    }
    std::string GetValue(size_t index) override {
        return ToString<T>(column_[index]);
    }

private:
    std::vector<T> column_;
};

class Batch {
public:
    Batch(size_t rows_number, std::vector<Type> types, std::vector<std::string> names);
    size_t GetSize();
    std::vector<Type> GetTypes();
    std::vector<std::string> GetNames();
    void SetValue(size_t row_index, size_t column_index, std::string value);
    std::string GetValue(size_t index_row, size_t index_column);

private:
    std::vector<std::unique_ptr<Column>> columns_;
    std::vector<std::string> names_;
    std::vector<Type> types_;
};

class Table {
public:
    Table(const std::string& file_input, const std::string& file_output);
    ~Table();
    void ReadBatch();
    void WriteBatch();

private:
    std::ifstream fin_;
    std::ofstream fout_;
    size_t rows_number_;
    size_t columns_number_;
    size_t column_batch_index_;
    std::unique_ptr<Batch> batch_;
};