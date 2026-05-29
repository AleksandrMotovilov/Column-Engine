#include "src/execution/aggregation_functions.h"

void CountRowsAggregation::Update(std::shared_ptr<Batch> batch) {
    count_ = static_cast<int64_t>(batch->GetRowsNumber());
}

std::string CountRowsAggregation::GetResult() const {
    return ToString<int64_t>(count_);
}

Type CountRowsAggregation::GetType() const {
    return Type::Int64;
}

std::string CountRowsAggregation::GetName() const {
    return "count(*)";
}

CountDistinctAggregation::CountDistinctAggregation(std::string column_name) {
    column_name_ = std::move(column_name);
}

void CountDistinctAggregation::Update(std::shared_ptr<Batch> batch) {
    size_t index = batch->FindColumn(column_name_);
    if (index == batch->GetColumnsNumber()) {
        return;
    }
    count_ = batch->GetColumn(index)->GetCountDistinct();
}

std::string CountDistinctAggregation::GetResult() const {
    return ToString<int64_t>(count_);
}

Type CountDistinctAggregation::GetType() const {
    return Type::Int64;
}

std::string CountDistinctAggregation::GetName() const {
    return "count(distinct " + column_name_ + ")";
}

SumAggregation::SumAggregation(std::string column_name) {
    column_name_ = std::move(column_name);
}

void SumAggregation::Update(std::shared_ptr<Batch> batch) {
    size_t index = batch->FindColumn(column_name_);
    if (index == batch->GetColumnsNumber()) {
        return;
    }
    sum_ = batch->GetColumn(index)->GetSum();
}

std::string SumAggregation::GetResult() const {
    return ToString<int64_t>(static_cast<int64_t>(sum_));
}

Type SumAggregation::GetType() const {
    return Type::Int64;
}

std::string SumAggregation::GetName() const {
    return "sum(" + column_name_ + ")";
}

AvgAggregation::AvgAggregation(std::string column_name) {
    column_name_ = std::move(column_name);
}

void AvgAggregation::Update(std::shared_ptr<Batch> batch) {
    size_t index = batch->FindColumn(column_name_);
    if (index == batch->GetColumnsNumber()) {
        return;
    }
    sum_ = batch->GetColumn(index)->GetSum();
    count_ = static_cast<__int128>(batch->GetColumn(index)->GetSize());
}

std::string AvgAggregation::GetResult() const {
    if (count_ == 0) {
        return "0";
    }
    return ToString<int64_t>(static_cast<int64_t>(sum_ / count_));
}

Type AvgAggregation::GetType() const {
    return Type::Int64;
}

std::string AvgAggregation::GetName() const {
    return "avg(" + column_name_ + ")";
}

MinAggregation::MinAggregation(std::string column_name) {
    column_name_ = std::move(column_name);
}

void MinAggregation::Update(std::shared_ptr<Batch> batch) {
    size_t index = batch->FindColumn(column_name_);
    if (index == batch->GetColumnsNumber()) {
        return;
    }
    column_type_ = batch->GetTypes()[index];
    min_value_ = batch->GetColumn(index)->GetMin();
}

std::string MinAggregation::GetResult() const {
    return min_value_;
}

Type MinAggregation::GetType() const {
    return column_type_;
}

std::string MinAggregation::GetName() const {
    return "min(" + column_name_ + ")";
}

MaxAggregation::MaxAggregation(std::string column_name) {
    column_name_ = std::move(column_name);
}

void MaxAggregation::Update(std::shared_ptr<Batch> batch) {
    size_t index = batch->FindColumn(column_name_);
    if (index == batch->GetColumnsNumber()) {
        return;
    }
    column_type_ = batch->GetTypes()[index];
    max_value_ = batch->GetColumn(index)->GetMax();
}

std::string MaxAggregation::GetResult() const {
    return max_value_;
}

Type MaxAggregation::GetType() const {
    return column_type_;
}

std::string MaxAggregation::GetName() const {
    return "max(" + column_name_ + ")";
}

SumWithOffsetAggregation::SumWithOffsetAggregation(std::string column_name, int64_t offset) {
    column_name_ = std::move(column_name);
    offset_ = offset;
}

void SumWithOffsetAggregation::Update(std::shared_ptr<Batch> batch) {
    size_t index = batch->FindColumn(column_name_);
    if (index == batch->GetColumnsNumber()) {
        return;
    }
    __int128 sum = batch->GetColumn(index)->GetSum();
    __int128 count = batch->GetColumn(index)->GetSize();
    sum_ = sum + static_cast<__int128>(offset_) * count;
}

std::string SumWithOffsetAggregation::GetResult() const {
    return ToString<int64_t>(static_cast<int64_t>(sum_));
}

Type SumWithOffsetAggregation::GetType() const {
    return Type::Int64;
}

std::string SumWithOffsetAggregation::GetName() const {
    return "sum(" + column_name_ + "+" + std::to_string(offset_) + ")";
}
