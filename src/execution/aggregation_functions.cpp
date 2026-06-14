#include "src/execution/aggregation_functions.h"

void CountRowsAggregation::Update(std::shared_ptr<Batch> batch) {
    count_ += static_cast<int64_t>(batch->GetRowsNumber());
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


CountDistinctAggregationTyped<Date>::CountDistinctAggregationTyped(std::string column_name) {
    column_name_ = std::move(column_name);
}

void CountDistinctAggregationTyped<Date>::Update(std::shared_ptr<Batch> batch) {
    size_t index = batch->FindColumn(column_name_);
    if (index == batch->GetColumnsNumber()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: CountDistinctAggregation");
    }
    const std::vector<Date>& data = dynamic_cast<const ColumnTyped<Date>&>(*batch->GetColumn(index)).GetData();
    for (const Date& val : data) {
        values_.insert(val.GetValue());
    }
}

std::string CountDistinctAggregationTyped<Date>::GetResult() const {
    return ToString<int64_t>(static_cast<int64_t>(values_.size()));
}

Type CountDistinctAggregationTyped<Date>::GetType() const {
    return Type::Int64;
}

std::string CountDistinctAggregationTyped<Date>::GetName() const {
    return "count(distinct " + column_name_ + ")";
}

CountDistinctAggregationTyped<Timestamp>::CountDistinctAggregationTyped(std::string column_name) {
    column_name_ = std::move(column_name);
}

void CountDistinctAggregationTyped<Timestamp>::Update(std::shared_ptr<Batch> batch) {
    size_t index = batch->FindColumn(column_name_);
    if (index == batch->GetColumnsNumber()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: CountDistinctAggregation");
    }
    const std::vector<Timestamp>& data = dynamic_cast<const ColumnTyped<Timestamp>&>(*batch->GetColumn(index)).GetData();
    for (const Timestamp& val : data) {
        values_.insert(val.GetValue());
    }
}

std::string CountDistinctAggregationTyped<Timestamp>::GetResult() const {
    return ToString<int64_t>(static_cast<int64_t>(values_.size()));
}

Type CountDistinctAggregationTyped<Timestamp>::GetType() const {
    return Type::Int64;
}

std::string CountDistinctAggregationTyped<Timestamp>::GetName() const {
    return "count(distinct " + column_name_ + ")";
}

CountDistinctAggregation::CountDistinctAggregation(std::string column_name) {
    column_name_ = std::move(column_name);
}

void CountDistinctAggregation::Update(std::shared_ptr<Batch> batch) {
    if (!function_) {
        size_t index = batch->FindColumn(column_name_);
        if (index == batch->GetColumnsNumber()) {
            throw std::runtime_error("Column not found: " + column_name_ + " :: CountDistinctAggregation");
        }
        switch (batch->GetType(index)) {
            case Type::Int16:
                function_ = std::make_shared<CountDistinctAggregationTyped<int16_t>>(column_name_);
                break;
            case Type::Int32:
                function_ = std::make_shared<CountDistinctAggregationTyped<int32_t>>(column_name_);
                break;
            case Type::Int64:
                function_ = std::make_shared<CountDistinctAggregationTyped<int64_t>>(column_name_);
                break;
            case Type::Float:
                function_ = std::make_shared<CountDistinctAggregationTyped<float>>(column_name_);
                break;
            case Type::Double:
                function_ = std::make_shared<CountDistinctAggregationTyped<double>>(column_name_);
                break;
            case Type::Date:
                function_ = std::make_shared<CountDistinctAggregationTyped<Date>>(column_name_);
                break;
            case Type::Timestamp:
                function_ = std::make_shared<CountDistinctAggregationTyped<Timestamp>>(column_name_);
                break;
            case Type::Char:
                function_ = std::make_shared<CountDistinctAggregationTyped<char>>(column_name_);
                break;
            case Type::String:
                function_ = std::make_shared<CountDistinctAggregationTyped<std::string>>(column_name_);
                break;
            default:
                throw std::runtime_error("Unsupported type :: CountDistinctAggregation");
        }
    }
    function_->Update(batch);
}

std::string CountDistinctAggregation::GetResult() const {
    if (!function_) {
        return "0";
    }
    return function_->GetResult();
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
        throw std::runtime_error("Column not found: " + column_name_ + " :: SumAggregation");
    }
    sum_ += batch->GetColumn(index)->GetSum();
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
        throw std::runtime_error("Column not found: " + column_name_ + " :: AvgAggregation");
    }
    sum_ += batch->GetColumn(index)->GetSum();
    count_ += static_cast<__int128>(batch->GetColumn(index)->GetSize());
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
    if (!function_) {
        size_t index = batch->FindColumn(column_name_);
        if (index == batch->GetColumnsNumber()) {
            throw std::runtime_error("Column not found: " + column_name_ + " :: MinAggregation");
        }
        switch (batch->GetType(index)) {
            case Type::Int16:
                function_ = std::make_shared<MinAggregationTyped<int16_t>>(column_name_);
                break;
            case Type::Int32:
                function_ = std::make_shared<MinAggregationTyped<int32_t>>(column_name_);
                break;
            case Type::Int64:
                function_ = std::make_shared<MinAggregationTyped<int64_t>>(column_name_);
                break;
            case Type::Float:
                function_ = std::make_shared<MinAggregationTyped<float>>(column_name_);
                break;
            case Type::Double:
                function_ = std::make_shared<MinAggregationTyped<double>>(column_name_);
                break;
            case Type::Date:
                function_ = std::make_shared<MinAggregationTyped<Date>>(column_name_);
                break;
            case Type::Timestamp:
                function_ = std::make_shared<MinAggregationTyped<Timestamp>>(column_name_);
                break;
            case Type::Char:
                function_ = std::make_shared<MinAggregationTyped<char>>(column_name_);
                break;
            case Type::String:
                function_ = std::make_shared<MinAggregationTyped<std::string>>(column_name_);
                break;
            default:
                throw std::runtime_error("Unsupported type :: MinAggregation");
        }
    }
    function_->Update(batch);
}

std::string MinAggregation::GetResult() const {
    if (!function_) {
        return "";
    }
    return function_->GetResult();
}

Type MinAggregation::GetType() const {
    if (!function_) {
        return Type::String;
    }
    return function_->GetType();
}

std::string MinAggregation::GetName() const {
    return "min(" + column_name_ + ")";
}

MaxAggregation::MaxAggregation(std::string column_name) {
    column_name_ = std::move(column_name);
}

void MaxAggregation::Update(std::shared_ptr<Batch> batch) {
    if (!function_) {
        size_t index = batch->FindColumn(column_name_);
        if (index == batch->GetColumnsNumber()) {
            throw std::runtime_error("Column not found: " + column_name_ + " :: MaxAggregation");
        }
        switch (batch->GetType(index)) {
            case Type::Int16:
                function_ = std::make_shared<MaxAggregationTyped<int16_t>>(column_name_);
                break;
            case Type::Int32:
                function_ = std::make_shared<MaxAggregationTyped<int32_t>>(column_name_);
                break;
            case Type::Int64:
                function_ = std::make_shared<MaxAggregationTyped<int64_t>>(column_name_);
                break;
            case Type::Float:
                function_ = std::make_shared<MaxAggregationTyped<float>>(column_name_);
                break;
            case Type::Double:
                function_ = std::make_shared<MaxAggregationTyped<double>>(column_name_);
                break;
            case Type::Date:
                function_ = std::make_shared<MaxAggregationTyped<Date>>(column_name_);
                break;
            case Type::Timestamp:
                function_ = std::make_shared<MaxAggregationTyped<Timestamp>>(column_name_);
                break;
            case Type::Char:
                function_ = std::make_shared<MaxAggregationTyped<char>>(column_name_);
                break;
            case Type::String:
                function_ = std::make_shared<MaxAggregationTyped<std::string>>(column_name_);
                break;
            default:
                throw std::runtime_error("Unsupported type :: MaxAggregation");
        }
    }
    function_->Update(batch);
}

std::string MaxAggregation::GetResult() const {
    if (!function_) {
        return "";
    }
    return function_->GetResult();
}

Type MaxAggregation::GetType() const {
    if (!function_) {
        return Type::String;
    }
    return function_->GetType();
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
        throw std::runtime_error("Column not found: " + column_name_ + " :: SumWithOffsetAggregation");
    }
    __int128 sum = batch->GetColumn(index)->GetSum();
    __int128 count = batch->GetColumn(index)->GetSize();
    sum_ += sum + static_cast<__int128>(offset_) * count;
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
