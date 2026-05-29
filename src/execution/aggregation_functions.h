#pragma once

#include <memory>
#include <string>
#include "src/batch.h"

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

class CountDistinctAggregation : public AggregationFunction {
public:
    explicit CountDistinctAggregation(std::string column_name);
    void Update(std::shared_ptr<Batch> batch) override;
    std::string GetResult() const override;
    Type GetType() const override;
    std::string GetName() const override;

private:
    std::string column_name_;
    int64_t count_ = 0;
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

class MinAggregation : public AggregationFunction {
public:
    explicit MinAggregation(std::string column_name);
    void Update(std::shared_ptr<Batch> batch) override;
    std::string GetResult() const override;
    Type GetType() const override;
    std::string GetName() const override;

private:
    std::string column_name_;
    std::string min_value_;
    Type column_type_ = Type::Int64;
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
    std::string max_value_;
    Type column_type_ = Type::Int64;
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
