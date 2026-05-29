#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <vector>
#include "src/types.h"

class Column {
public:
    virtual ~Column() = default;
    virtual size_t GetSize() const = 0;
    virtual std::string GetMin() const = 0;
    virtual std::string GetMax() const = 0;
    virtual __int128 GetSum() const = 0;
    virtual int64_t GetCountDistinct() const = 0;
};

template<typename T>
class ColumnTyped : public Column {
public:
    explicit ColumnTyped(std::vector<T>&& data) {
        column_ = std::move(data);
        if (column_.empty()) {
            return;
        }
        std::unordered_set<T> distinct;
        T min_val = column_[0];
        T max_val = column_[0];
        __int128 sum = 0;
        for (const T& val : column_) {
            distinct.insert(val);
            if (val < min_val) {
                min_val = val;
            }
            if (val > max_val) {
                max_val = val;
            }
            if constexpr (std::is_arithmetic_v<T>) {
                sum += static_cast<__int128>(val);
            }
        }
        count_distinct_ = static_cast<int64_t>(distinct.size());
        min_val_ = min_val;
        max_val_ = max_val;
        sum_ = sum;
    }

    size_t GetSize() const override {
        return column_.size();
    }

    std::string GetMin() const override {
        if (column_.empty()) {
            return "";
        }
        return ToString<T>(min_val_);
    }

    std::string GetMax() const override {
        if (column_.empty()) {
            return "";
        }
        return ToString<T>(max_val_);
    }

    __int128 GetSum() const override {
        return sum_;
    }

    int64_t GetCountDistinct() const override {
        return count_distinct_;
    }

    const std::vector<T>& GetData() const {
        return column_;
    }

    std::vector<T> GetVector() {
        return std::move(column_);
    }

private:
    std::vector<T> column_;
    int64_t count_distinct_ = 0;
    T min_val_ = T{};
    T max_val_ = T{};
    __int128 sum_ = 0;
};

template<>
ColumnTyped<Date>::ColumnTyped(std::vector<Date>&& data);

template<>
ColumnTyped<Timestamp>::ColumnTyped(std::vector<Timestamp>&& data);

std::string GetStringValueAt(std::shared_ptr<Column> column, Type column_type, size_t index);

std::shared_ptr<Column> CopyRowsTyped(std::shared_ptr<Column> column, Type column_type, const std::vector<size_t>& indices);

std::shared_ptr<Column> MakeSingleValueColumn(Type column_type, const std::string& value);

std::shared_ptr<Column> MakeColumnFromStrings(Type column_type, const std::vector<std::string>& strs);
