#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include "src/kernel/batch.h"
#include "src/kernel/column.h"

__int128 ComputeColumnSum(std::shared_ptr<Column> column, Type type);

class AggregationFunction {
public:
    virtual ~AggregationFunction() = default;
    virtual void Update(std::shared_ptr<Batch> batch) = 0;
    virtual void AppendResultBytes(std::vector<char>& buf) const = 0;
    virtual Type GetType() const = 0;
    virtual std::string GetName() const = 0;
    virtual std::string GetNeededColumnName() const = 0;
};

class CountRowsAggregation : public AggregationFunction {
public:
    void Update(std::shared_ptr<Batch> batch) override;
    void AppendResultBytes(std::vector<char>& buf) const override;
    Type GetType() const override;
    std::string GetName() const override;
    std::string GetNeededColumnName() const override;

private:
    int64_t count_ = 0;
};

template<typename T>
class CountDistinctAggregationTyped : public AggregationFunction {
public:
    explicit CountDistinctAggregationTyped(std::string column_name) {
        column_name_ = std::move(column_name);
    }

    void Update(std::shared_ptr<Batch> batch) override {
        size_t index = batch->FindColumn(column_name_);
        if (index == batch->GetColumnsNumber()) {
            throw std::runtime_error("Column not found: " + column_name_ + " :: CountDistinctAggregation");
        }
        const std::vector<T>& column_data = static_cast<const ColumnTyped<T>&>(*batch->GetColumn(index)).GetData();
        for (const T& value : column_data) {
            values_.insert(value);
        }
    }

    void AppendResultBytes(std::vector<char>& buf) const override {
        int64_t count = static_cast<int64_t>(values_.size());
        const char* ptr = reinterpret_cast<const char*>(&count);
        buf.insert(buf.end(), ptr, ptr + sizeof(int64_t));
    }

    Type GetType() const override {
        return Type::Int64;
    }

    std::string GetName() const override {
        return "count(distinct " + column_name_ + ")";
    }

    std::string GetNeededColumnName() const override {
        return column_name_;
    }

private:
    std::string column_name_;
    std::unordered_set<T> values_;
};

template<>
class CountDistinctAggregationTyped<Date> : public AggregationFunction {
public:
    explicit CountDistinctAggregationTyped(std::string column_name);
    void Update(std::shared_ptr<Batch> batch) override;
    void AppendResultBytes(std::vector<char>& buf) const override;
    Type GetType() const override;
    std::string GetName() const override;
    std::string GetNeededColumnName() const override;

private:
    std::string column_name_;
    std::unordered_set<int32_t> values_;
};

template<>
class CountDistinctAggregationTyped<Timestamp> : public AggregationFunction {
public:
    explicit CountDistinctAggregationTyped(std::string column_name);
    void Update(std::shared_ptr<Batch> batch) override;
    void AppendResultBytes(std::vector<char>& buf) const override;
    Type GetType() const override;
    std::string GetName() const override;
    std::string GetNeededColumnName() const override;

private:
    std::string column_name_;
    std::unordered_set<int64_t> values_;
};

class CountDistinctAggregation : public AggregationFunction {
public:
    explicit CountDistinctAggregation(std::string column_name);
    void Update(std::shared_ptr<Batch> batch) override;
    void AppendResultBytes(std::vector<char>& buf) const override;
    Type GetType() const override;
    std::string GetName() const override;
    std::string GetNeededColumnName() const override;

private:
    std::string column_name_;
    std::shared_ptr<AggregationFunction> function_;
};

class SumAggregation : public AggregationFunction {
public:
    explicit SumAggregation(std::string column_name);
    void Update(std::shared_ptr<Batch> batch) override;
    void AppendResultBytes(std::vector<char>& buf) const override;
    Type GetType() const override;
    std::string GetName() const override;
    std::string GetNeededColumnName() const override;

private:
    std::string column_name_;
    __int128 sum_ = 0;
};

class SumWithOffsetAggregation : public AggregationFunction {
public:
    SumWithOffsetAggregation(std::string column_name, int64_t offset);
    void Update(std::shared_ptr<Batch> batch) override;
    void AppendResultBytes(std::vector<char>& buf) const override;
    Type GetType() const override;
    std::string GetName() const override;
    std::string GetNeededColumnName() const override;

private:
    std::string column_name_;
    int64_t offset_;
    __int128 sum_ = 0;
};

class AvgAggregation : public AggregationFunction {
public:
    explicit AvgAggregation(std::string column_name);
    void Update(std::shared_ptr<Batch> batch) override;
    void AppendResultBytes(std::vector<char>& buf) const override;
    Type GetType() const override;
    std::string GetName() const override;
    std::string GetNeededColumnName() const override;

private:
    std::string column_name_;
    __int128 sum_ = 0;
    __int128 count_ = 0;
};

template<typename T>
class MinAggregationTyped : public AggregationFunction {
public:
    explicit MinAggregationTyped(std::string column_name) {
        column_name_ = std::move(column_name);
    }

    void Update(std::shared_ptr<Batch> batch) override {
        size_t index = batch->FindColumn(column_name_);
        if (index == batch->GetColumnsNumber()) {
            throw std::runtime_error("Column not found: " + column_name_ + " :: MinAggregation");
        }
        const std::vector<T>& data = static_cast<const ColumnTyped<T>&>(*batch->GetColumn(index)).GetData();
        for (const T& value : data) {
            if (!min_value_.has_value() || value < *min_value_) {
                min_value_ = value;
            }
        }
    }

    void AppendResultBytes(std::vector<char>& buf) const override {
        T value = T{};
        if (min_value_.has_value()) {
            value = *min_value_;
        }
        const char* ptr = reinterpret_cast<const char*>(&value);
        buf.insert(buf.end(), ptr, ptr + sizeof(T));
    }

    Type GetType() const override {
        return TypeOf<T>();
    }

    std::string GetName() const override {
        return "min(" + column_name_ + ")";
    }

    std::string GetNeededColumnName() const override {
        return column_name_;
    }

private:
    std::string column_name_;
    std::optional<T> min_value_;
};

template<>
void MinAggregationTyped<std::string>::AppendResultBytes(std::vector<char>& buf) const;

template<>
void MinAggregationTyped<Date>::AppendResultBytes(std::vector<char>& buf) const;

template<>
void MinAggregationTyped<Timestamp>::AppendResultBytes(std::vector<char>& buf) const;

class MinAggregation : public AggregationFunction {
public:
    explicit MinAggregation(std::string column_name);
    void Update(std::shared_ptr<Batch> batch) override;
    void AppendResultBytes(std::vector<char>& buf) const override;
    Type GetType() const override;
    std::string GetName() const override;
    std::string GetNeededColumnName() const override;

private:
    std::string column_name_;
    std::shared_ptr<AggregationFunction> function_;
};

template<typename T>
class MaxAggregationTyped : public AggregationFunction {
public:
    explicit MaxAggregationTyped(std::string column_name) {
        column_name_ = std::move(column_name);
    }

    void Update(std::shared_ptr<Batch> batch) override {
        size_t index = batch->FindColumn(column_name_);
        if (index == batch->GetColumnsNumber()) {
            throw std::runtime_error("Column not found: " + column_name_ + " :: MaxAggregation");
        }
        const std::vector<T>& data = static_cast<const ColumnTyped<T>&>(*batch->GetColumn(index)).GetData();
        for (const T& value : data) {
            if (!max_value_.has_value() || value > *max_value_) {
                max_value_ = value;
            }
        }
    }

    void AppendResultBytes(std::vector<char>& buf) const override {
        T value = T{};
        if (max_value_.has_value()) {
            value = *max_value_;
        }
        const char* ptr = reinterpret_cast<const char*>(&value);
        buf.insert(buf.end(), ptr, ptr + sizeof(T));
    }

    Type GetType() const override {
        return TypeOf<T>();
    }

    std::string GetName() const override {
        return "max(" + column_name_ + ")";
    }

    std::string GetNeededColumnName() const override {
        return column_name_;
    }

private:
    std::string column_name_;
    std::optional<T> max_value_;
};

template<>
void MaxAggregationTyped<Date>::AppendResultBytes(std::vector<char>& buf) const;

template<>
void MaxAggregationTyped<Timestamp>::AppendResultBytes(std::vector<char>& buf) const;

template<>
void MaxAggregationTyped<std::string>::AppendResultBytes(std::vector<char>& buf) const;

class MaxAggregation : public AggregationFunction {
public:
    explicit MaxAggregation(std::string column_name);
    void Update(std::shared_ptr<Batch> batch) override;
    void AppendResultBytes(std::vector<char>& buf) const override;
    Type GetType() const override;
    std::string GetName() const override;
    std::string GetNeededColumnName() const override;

private:
    std::string column_name_;
    std::shared_ptr<AggregationFunction> function_;
};
