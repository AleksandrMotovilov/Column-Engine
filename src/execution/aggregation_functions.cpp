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
    std::vector<std::string> columns_names = batch->GetNames();
    size_t column_index = columns_names.size();
    for (size_t i = 0; i < columns_names.size(); i++) {
        if (columns_names[i] == column_name_) {
            column_index = i;
            break;
        }
    }
    if (column_index == columns_names.size()) {
        return;
    }
    for (size_t i = 0; i < batch->GetRowsNumber(); i++) {
        distinct_values_.insert(batch->GetValue(i, column_index));
    }
}

std::string CountDistinctAggregation::GetResult() const {
    return ToString<int64_t>(static_cast<int64_t>(distinct_values_.size()));
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
    std::vector<std::string> columns_names = batch->GetNames();
    size_t column_index = columns_names.size();
    for (size_t i = 0; i < columns_names.size(); i++) {
        if (columns_names[i] == column_name_) {
            column_index = i;
            break;
        }
    }
    if (column_index == columns_names.size()) {
        return;
    }
    __int128 sum = 0;
    for (size_t i = 0; i < batch->GetRowsNumber(); i++) {
        sum += static_cast<__int128>(FromString<int64_t>(batch->GetValue(i, column_index)));
    }
    sum_ = sum;
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
    std::vector<std::string> columns_names = batch->GetNames();
    size_t column_index = columns_names.size();
    for (size_t i = 0; i < columns_names.size(); i++) {
        if (columns_names[i] == column_name_) {
            column_index = i;
            break;
        }
    }
    if (column_index == columns_names.size()) {
        return;
    }
    __int128 sum = 0;
    int64_t count = 0;
    for (size_t i = 0; i < batch->GetRowsNumber(); i++) {
        sum += static_cast<__int128>(FromString<int64_t>(batch->GetValue(i, column_index)));
        count++;
    }
    sum_ = sum;
    count_ = count;
}

std::string AvgAggregation::GetResult() const {
    if (count_ == 0) {
        return "0";
    }
    return ToString<int64_t>(static_cast<int64_t>(sum_ / static_cast<__int128>(count_)));
}

Type AvgAggregation::GetType() const {
    return Type::Int64;
}

std::string AvgAggregation::GetName() const {
    return "avg(" + column_name_ + ")";
}

MinAggregation::MinAggregation(std::string column_name) {
    column_name_ = std::move(column_name);
    has_value_ = false;
    column_type_ = Type::Int64;
}

void MinAggregation::Update(std::shared_ptr<Batch> batch) {
    std::vector<std::string> columns_names = batch->GetNames();
    size_t column_index = columns_names.size();
    for (size_t i = 0; i < columns_names.size(); i++) {
        if (columns_names[i] == column_name_) {
            column_index = i;
            break;
        }
    }
    if (column_index == columns_names.size()) {
        return;
    }
    if (!has_value_) {
        column_type_ = batch->GetTypes()[column_index];
    }
    for (size_t i = 0; i < batch->GetRowsNumber(); i++) {
        std::string value = batch->GetValue(i, column_index);
        if (!has_value_ || IsLess(value, min_value_)) {
            min_value_ = value;
            has_value_ = true;
        }
    }
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

bool MinAggregation::IsLess(const std::string& a, const std::string& b) {
    switch (column_type_) {
        case Type::Int16:
            return FromString<int16_t>(a) < FromString<int16_t>(b);
        case Type::Int32:
            return FromString<int32_t>(a) < FromString<int32_t>(b);
        case Type::Int64:
            return FromString<int64_t>(a) < FromString<int64_t>(b);
        case Type::Float:
            return FromString<float>(a) < FromString<float>(b);
        case Type::Double:
            return FromString<double>(a) < FromString<double>(b);
        case Type::Date:
            return FromString<Date>(a).days < FromString<Date>(b).days;
        case Type::Timestamp:
            return FromString<Timestamp>(a).seconds < FromString<Timestamp>(b).seconds;
        default:
            return a < b;
    }
}

MaxAggregation::MaxAggregation(std::string column_name) {
    column_name_ = std::move(column_name);
    has_value_ = false;
    column_type_ = Type::Int64;
}

void MaxAggregation::Update(std::shared_ptr<Batch> batch) {
    std::vector<std::string> columns_names = batch->GetNames();
    size_t column_index = columns_names.size();
    for (size_t i = 0; i < columns_names.size(); i++) {
        if (columns_names[i] == column_name_) {
            column_index = i;
            break;
        }
    }
    if (column_index == columns_names.size()) {
        return;
    }
    if (!has_value_) {
        column_type_ = batch->GetTypes()[column_index];
    }
    for (size_t i = 0; i < batch->GetRowsNumber(); i++) {
        std::string value = batch->GetValue(i, column_index);
        if (!has_value_ || IsGreater(value, max_value_)) {
            max_value_ = value;
            has_value_ = true;
        }
    }
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

bool MaxAggregation::IsGreater(const std::string& a, const std::string& b) {
    switch (column_type_) {
        case Type::Int16:
            return FromString<int16_t>(a) > FromString<int16_t>(b);
        case Type::Int32:
            return FromString<int32_t>(a) > FromString<int32_t>(b);
        case Type::Int64:
            return FromString<int64_t>(a) > FromString<int64_t>(b);
        case Type::Float:
            return FromString<float>(a) > FromString<float>(b);
        case Type::Double:
            return FromString<double>(a) > FromString<double>(b);
        case Type::Date:
            return FromString<Date>(a).days > FromString<Date>(b).days;
        case Type::Timestamp:
            return FromString<Timestamp>(a).seconds > FromString<Timestamp>(b).seconds;
        default:
            return a > b;
    }
}

SumWithOffsetAggregation::SumWithOffsetAggregation(std::string column_name, int64_t offset) {
    column_name_ = std::move(column_name);
    offset_ = offset;
}

void SumWithOffsetAggregation::Update(std::shared_ptr<Batch> batch) {
    std::vector<std::string> columns_names = batch->GetNames();
    size_t column_index = columns_names.size();
    for (size_t i = 0; i < columns_names.size(); i++) {
        if (columns_names[i] == column_name_) {
            column_index = i;
            break;
        }
    }
    if (column_index == columns_names.size()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: SumWithOffsetAggregation");
    }
    __int128 sum = 0;
    size_t rows = batch->GetRowsNumber();
    for (size_t i = 0; i < rows; i++) {
        sum += static_cast<__int128>(FromString<int64_t>(batch->GetValue(i, column_index))) + offset_;
    }
    sum_ = sum;
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
