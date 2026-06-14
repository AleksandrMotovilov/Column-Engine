#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include "src/kernel/types.h"

class Column {
public:
    virtual ~Column() = default;
    virtual size_t GetSize() const = 0;
    virtual void AppendMinBytes(std::vector<char>& buf) const = 0;
    virtual void AppendMaxBytes(std::vector<char>& buf) const = 0;
    virtual __int128 GetSum() const = 0;
};

template<typename T>
class ColumnTyped : public Column {
public:
    explicit ColumnTyped(std::vector<T>&& data) {
        column_ = std::move(data);
        if (column_.empty()) {
            return;
        }
        T min_value = column_[0];
        T max_value = column_[0];
        __int128 sum = 0;
        for (const T& value : column_) {
            if (value < min_value) {
                min_value = value;
            }
            if (value > max_value) {
                max_value = value;
            }
            if constexpr (std::is_arithmetic_v<T>) {
                sum += static_cast<__int128>(value);
            }
        }
        min_value_ = min_value;
        max_value_ = max_value;
        sum_ = sum;
    }

    size_t GetSize() const override {
        return column_.size();
    }

    void AppendMinBytes(std::vector<char>& buf) const override {
        const char* ptr = reinterpret_cast<const char*>(&min_value_);
        buf.insert(buf.end(), ptr, ptr + sizeof(T));
    }

    void AppendMaxBytes(std::vector<char>& buf) const override {
        const char* ptr = reinterpret_cast<const char*>(&max_value_);
        buf.insert(buf.end(), ptr, ptr + sizeof(T));
    }

    __int128 GetSum() const override {
        return sum_;
    }

    const std::vector<T>& GetData() const {
        return column_;
    }

    std::vector<T> GetVector() {
        return std::move(column_);
    }

private:
    std::vector<T> column_;
    T min_value_ = T{};
    T max_value_ = T{};
    __int128 sum_ = 0;
};

template<>
ColumnTyped<Date>::ColumnTyped(std::vector<Date>&& data);

template<>
ColumnTyped<Timestamp>::ColumnTyped(std::vector<Timestamp>&& data);

template<>
void ColumnTyped<Date>::AppendMinBytes(std::vector<char>& buf) const;

template<>
void ColumnTyped<Date>::AppendMaxBytes(std::vector<char>& buf) const;

template<>
void ColumnTyped<Timestamp>::AppendMinBytes(std::vector<char>& buf) const;

template<>
void ColumnTyped<Timestamp>::AppendMaxBytes(std::vector<char>& buf) const;

template<>
void ColumnTyped<std::string>::AppendMinBytes(std::vector<char>& buf) const;

template<>
void ColumnTyped<std::string>::AppendMaxBytes(std::vector<char>& buf) const;
