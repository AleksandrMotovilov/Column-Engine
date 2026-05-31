#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include "src/kernel/batch.h"
#include "src/kernel/column.h"

class AggregationFunction {
public:
    virtual ~AggregationFunction() = default;
    virtual void Update(std::shared_ptr<Batch> batch) = 0;
    virtual std::string GetResult() const = 0;
    virtual Type GetType() const = 0;
    virtual std::string GetName() const = 0;
};

class CountRowsAggregation : public AggregationFunction {
public:
    void Update(std::shared_ptr<Batch> batch) override;
    std::string GetResult() const override;
    Type GetType() const override;
    std::string GetName() const override;

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
        const std::vector<T>& data = dynamic_cast<const ColumnTyped<T>&>(*batch->GetColumn(index)).GetData();
        for (const T& val : data) {
            values_.insert(val);
        }
    }

    std::string GetResult() const override {
        return ToString<int64_t>(static_cast<int64_t>(values_.size()));
    }

    Type GetType() const override {
        return Type::Int64;
    }

    std::string GetName() const override {
        return "count(distinct " + column_name_ + ")";
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
    std::string GetResult() const override;
    Type GetType() const override;
    std::string GetName() const override;

private:
    std::string column_name_;
    std::unordered_set<int32_t> values_;
};

template<>
class CountDistinctAggregationTyped<Timestamp> : public AggregationFunction {
public:
    explicit CountDistinctAggregationTyped(std::string column_name);
    void Update(std::shared_ptr<Batch> batch) override;
    std::string GetResult() const override;
    Type GetType() const override;
    std::string GetName() const override;

private:
    std::string column_name_;
    std::unordered_set<int64_t> values_;
};

class CountDistinctAggregation : public AggregationFunction {
public:
    explicit CountDistinctAggregation(std::string column_name);
    void Update(std::shared_ptr<Batch> batch) override;
    std::string GetResult() const override;
    Type GetType() const override;
    std::string GetName() const override;

private:
    std::string column_name_;
    std::shared_ptr<AggregationFunction> function_;
};

class SumAggregation : public AggregationFunction {
public:
    explicit SumAggregation(std::string column_name);
    void Update(std::shared_ptr<Batch> batch) override;
    std::string GetResult() const override;
    Type GetType() const override;
    std::string GetName() const override;

private:
    std::string column_name_;
    __int128 sum_ = 0;
};

class AvgAggregation : public AggregationFunction {
public:
    explicit AvgAggregation(std::string column_name);
    void Update(std::shared_ptr<Batch> batch) override;
    std::string GetResult() const override;
    Type GetType() const override;
    std::string GetName() const override;

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
        const ColumnTyped<T>& column = dynamic_cast<const ColumnTyped<T>&>(*batch->GetColumn(index));
        if (column.GetSize() == 0) {
            return;
        }
        const T& batch_min = column.GetMinValue();
        if (!min_val_.has_value() || batch_min < *min_val_) {
            min_val_ = batch_min;
        }
    }

    std::string GetResult() const override {
        if (!min_val_.has_value()) {
            return "";
        }
        return ToString<T>(*min_val_);
    }

    Type GetType() const override {
        return TypeOf<T>();
    }

    std::string GetName() const override {
        return "min(" + column_name_ + ")";
    }

private:
    std::string column_name_;
    std::optional<T> min_val_;
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
        const ColumnTyped<T>& column = dynamic_cast<const ColumnTyped<T>&>(*batch->GetColumn(index));
        if (column.GetSize() == 0) {
            return;
        }
        const T& batch_max = column.GetMaxValue();
        if (!max_val_.has_value() || batch_max > *max_val_) {
            max_val_ = batch_max;
        }
    }

    std::string GetResult() const override {
        if (!max_val_.has_value()) {
            return "";
        }
        return ToString<T>(*max_val_);
    }

    Type GetType() const override {
        return TypeOf<T>();
    }

    std::string GetName() const override {
        return "max(" + column_name_ + ")";
    }

private:
    std::string column_name_;
    std::optional<T> max_val_;
};

class MinAggregation : public AggregationFunction {
public:
    explicit MinAggregation(std::string column_name);
    void Update(std::shared_ptr<Batch> batch) override;
    std::string GetResult() const override;
    Type GetType() const override;
    std::string GetName() const override;

private:
    std::string column_name_;
    std::shared_ptr<AggregationFunction> function_;
};

class MaxAggregation : public AggregationFunction {
public:
    explicit MaxAggregation(std::string column_name);
    void Update(std::shared_ptr<Batch> batch) override;
    std::string GetResult() const override;
    Type GetType() const override;
    std::string GetName() const override;

private:
    std::string column_name_;
    std::shared_ptr<AggregationFunction> function_;
};

class SumWithOffsetAggregation : public AggregationFunction {
public:
    SumWithOffsetAggregation(std::string column_name, int64_t offset);
    void Update(std::shared_ptr<Batch> batch) override;
    std::string GetResult() const override;
    Type GetType() const override;
    std::string GetName() const override;

private:
    std::string column_name_;
    int64_t offset_;
    __int128 sum_ = 0;
};
