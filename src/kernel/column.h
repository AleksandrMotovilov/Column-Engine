#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "src/kernel/types.h"

class Column {
public:
    virtual ~Column() = default;
    virtual size_t GetSize() const = 0;
};

template<typename T>
class ColumnTyped : public Column {
public:
    explicit ColumnTyped(std::vector<T>&& data) {
        column_ = std::move(data);
    }

    size_t GetSize() const override {
        return column_.size();
    }

    const std::vector<T>& GetData() const {
        return column_;
    }

    std::vector<T> GetVector() {
        return std::move(column_);
    }

private:
    std::vector<T> column_;
};
