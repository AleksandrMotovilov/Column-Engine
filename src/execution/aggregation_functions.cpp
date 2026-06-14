#include "src/execution/aggregation_functions.h"

void CountRowsAggregation::Update(std::shared_ptr<Batch> batch) {
    count_ += static_cast<int64_t>(batch->GetRowsNumber());
}

void CountRowsAggregation::AppendResultBytes(std::vector<char>& buf) const {
    const char* ptr = reinterpret_cast<const char*>(&count_);
    buf.insert(buf.end(), ptr, ptr + sizeof(int64_t));
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
    const std::vector<Date>& column_data = dynamic_cast<const ColumnTyped<Date>&>(*batch->GetColumn(index)).GetData();
    for (const Date& value : column_data) {
        values_.insert(value.GetValue());
    }
}

void CountDistinctAggregationTyped<Date>::AppendResultBytes(std::vector<char>& buf) const {
    int64_t count = static_cast<int64_t>(values_.size());
    const char* ptr = reinterpret_cast<const char*>(&count);
    buf.insert(buf.end(), ptr, ptr + sizeof(int64_t));
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
    const std::vector<Timestamp>& column_data = dynamic_cast<const ColumnTyped<Timestamp>&>(*batch->GetColumn(index)).GetData();
    for (const Timestamp& value : column_data) {
        values_.insert(value.GetValue());
    }
}

void CountDistinctAggregationTyped<Timestamp>::AppendResultBytes(std::vector<char>& buf) const {
    int64_t count = static_cast<int64_t>(values_.size());
    const char* ptr = reinterpret_cast<const char*>(&count);
    buf.insert(buf.end(), ptr, ptr + sizeof(int64_t));
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

void CountDistinctAggregation::AppendResultBytes(std::vector<char>& buf) const {
    if (!function_) {
        int64_t zero = 0;
        const char* ptr = reinterpret_cast<const char*>(&zero);
        buf.insert(buf.end(), ptr, ptr + sizeof(int64_t));
        return;
    }
    function_->AppendResultBytes(buf);
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

void SumAggregation::AppendResultBytes(std::vector<char>& buf) const {
    int64_t value = static_cast<int64_t>(sum_);
    const char* ptr = reinterpret_cast<const char*>(&value);
    buf.insert(buf.end(), ptr, ptr + sizeof(int64_t));
}

Type SumAggregation::GetType() const {
    return Type::Int64;
}

std::string SumAggregation::GetName() const {
    return "sum(" + column_name_ + ")";
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

void SumWithOffsetAggregation::AppendResultBytes(std::vector<char>& buf) const {
    int64_t val = static_cast<int64_t>(sum_);
    const char* ptr = reinterpret_cast<const char*>(&val);
    buf.insert(buf.end(), ptr, ptr + sizeof(int64_t));
}

Type SumWithOffsetAggregation::GetType() const {
    return Type::Int64;
}

std::string SumWithOffsetAggregation::GetName() const {
    return "sum(" + column_name_ + "+" + std::to_string(offset_) + ")";
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

void AvgAggregation::AppendResultBytes(std::vector<char>& buf) const {
    int64_t value = 0;
    if (count_ > 0) {
        value = static_cast<int64_t>(sum_ / count_);
    }
    const char* ptr = reinterpret_cast<const char*>(&value);
    buf.insert(buf.end(), ptr, ptr + sizeof(int64_t));
}

Type AvgAggregation::GetType() const {
    return Type::Int64;
}

std::string AvgAggregation::GetName() const {
    return "avg(" + column_name_ + ")";
}

template<>
void MinAggregationTyped<std::string>::Update(std::shared_ptr<Batch> batch) {
    size_t index = batch->FindColumn(column_name_);
    if (index == batch->GetColumnsNumber()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: MinAggregation");
    }
    std::shared_ptr<Column> column = batch->GetColumn(index);
    if (column->GetSize() == 0) {
        return;
    }
    std::vector<char> buf;
    column->AppendMinBytes(buf);
    size_t len;
    std::memcpy(&len, buf.data(), sizeof(size_t));
    std::string batch_min(buf.data() + sizeof(size_t), len);
    if (!min_value_.has_value() || batch_min < *min_value_) {
        min_value_ = batch_min;
    }
}

template<>
void MinAggregationTyped<std::string>::AppendResultBytes(std::vector<char>& buf) const {
    std::string value;
    if (min_value_.has_value()) {
        value = *min_value_;
    }
    size_t len = value.size();
    const char* ptr = reinterpret_cast<const char*>(&len);
    buf.insert(buf.end(), ptr, ptr + sizeof(size_t));
    buf.insert(buf.end(), value.begin(), value.end());
}

template<>
void MinAggregationTyped<Date>::AppendResultBytes(std::vector<char>& buf) const {
    int32_t value = 0;
    if (min_value_.has_value()) {
        value = min_value_->GetValue();
    }
    const char* ptr = reinterpret_cast<const char*>(&value);
    buf.insert(buf.end(), ptr, ptr + sizeof(int32_t));
}

template<>
void MinAggregationTyped<Timestamp>::AppendResultBytes(std::vector<char>& buf) const {
    int64_t value = 0;
    if (min_value_.has_value()) {
        value = min_value_->GetValue();
    }
    const char* ptr = reinterpret_cast<const char*>(&value);
    buf.insert(buf.end(), ptr, ptr + sizeof(int64_t));
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

void MinAggregation::AppendResultBytes(std::vector<char>& buf) const {
    if (!function_) {
        size_t zero = 0;
        const char* ptr = reinterpret_cast<const char*>(&zero);
        buf.insert(buf.end(), ptr, ptr + sizeof(size_t));
        return;
    }
    function_->AppendResultBytes(buf);
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

template<>
void MaxAggregationTyped<std::string>::Update(std::shared_ptr<Batch> batch) {
    size_t index = batch->FindColumn(column_name_);
    if (index == batch->GetColumnsNumber()) {
        throw std::runtime_error("Column not found: " + column_name_ + " :: MaxAggregation");
    }
    std::shared_ptr<Column> column = batch->GetColumn(index);
    if (column->GetSize() == 0) {
        return;
    }
    std::vector<char> buf;
    column->AppendMaxBytes(buf);
    size_t len;
    std::memcpy(&len, buf.data(), sizeof(size_t));
    std::string batch_max(buf.data() + sizeof(size_t), len);
    if (!max_value_.has_value() || batch_max > *max_value_) {
        max_value_ = batch_max;
    }
}

template<>
void MaxAggregationTyped<Date>::AppendResultBytes(std::vector<char>& buf) const {
    int32_t value = 0;
    if (max_value_.has_value()) {
        value = max_value_->GetValue();
    }
    const char* ptr = reinterpret_cast<const char*>(&value);
    buf.insert(buf.end(), ptr, ptr + sizeof(int32_t));
}

template<>
void MaxAggregationTyped<Timestamp>::AppendResultBytes(std::vector<char>& buf) const {
    int64_t value = 0;
    if (max_value_.has_value()) {
        value = max_value_->GetValue();
    }
    const char* ptr = reinterpret_cast<const char*>(&value);
    buf.insert(buf.end(), ptr, ptr + sizeof(int64_t));
}

template<>
void MaxAggregationTyped<std::string>::AppendResultBytes(std::vector<char>& buf) const {
    std::string value;
    if (max_value_.has_value()) {
        value = *max_value_;
    }
    size_t len = value.size();
    const char* ptr = reinterpret_cast<const char*>(&len);
    buf.insert(buf.end(), ptr, ptr + sizeof(size_t));
    buf.insert(buf.end(), value.begin(), value.end());
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

void MaxAggregation::AppendResultBytes(std::vector<char>& buf) const {
    if (!function_) {
        size_t zero = 0;
        const char* ptr = reinterpret_cast<const char*>(&zero);
        buf.insert(buf.end(), ptr, ptr + sizeof(size_t));
        return;
    }
    function_->AppendResultBytes(buf);
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
