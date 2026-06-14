#include "src/kernel/column.h"

template<>
ColumnTyped<Date>::ColumnTyped(std::vector<Date>&& data) {
    column_ = std::move(data);
    if (column_.empty()) {
        return;
    }
    Date min_value = column_[0];
    Date max_value = column_[0];
    __int128 sum = 0;
    for (const Date& value : column_) {
        if (value < min_value) {
            min_value = value;
        }
        if (value > max_value) {
            max_value = value;
        }
        sum += static_cast<__int128>(value.GetValue());
    }
    min_value_ = min_value;
    max_value_ = max_value;
    sum_ = sum;
}

template<>
ColumnTyped<Timestamp>::ColumnTyped(std::vector<Timestamp>&& data) {
    column_ = std::move(data);
    if (column_.empty()) {
        return;
    }
    Timestamp min_value = column_[0];
    Timestamp max_value = column_[0];
    __int128 sum = 0;
    for (const Timestamp& value : column_) {
        if (value < min_value) {
            min_value = value;
        }
        if (value > max_value) {
            max_value = value;
        }
        sum += static_cast<__int128>(value.GetValue());
    }
    min_value_ = min_value;
    max_value_ = max_value;
    sum_ = sum;
}

template<>
void ColumnTyped<Date>::AppendMinBytes(std::vector<char>& buf) const {
    int32_t v = min_value_.GetValue();
    const char* ptr = reinterpret_cast<const char*>(&v);
    buf.insert(buf.end(), ptr, ptr + sizeof(int32_t));
}

template<>
void ColumnTyped<Date>::AppendMaxBytes(std::vector<char>& buf) const {
    int32_t v = max_value_.GetValue();
    const char* ptr = reinterpret_cast<const char*>(&v);
    buf.insert(buf.end(), ptr, ptr + sizeof(int32_t));
}

template<>
void ColumnTyped<Timestamp>::AppendMinBytes(std::vector<char>& buf) const {
    int64_t v = min_value_.GetValue();
    const char* ptr = reinterpret_cast<const char*>(&v);
    buf.insert(buf.end(), ptr, ptr + sizeof(int64_t));
}

template<>
void ColumnTyped<Timestamp>::AppendMaxBytes(std::vector<char>& buf) const {
    int64_t v = max_value_.GetValue();
    const char* ptr = reinterpret_cast<const char*>(&v);
    buf.insert(buf.end(), ptr, ptr + sizeof(int64_t));
}

template<>
void ColumnTyped<std::string>::AppendMinBytes(std::vector<char>& buf) const {
    size_t len = min_value_.size();
    const char* ptr = reinterpret_cast<const char*>(&len);
    buf.insert(buf.end(), ptr, ptr + sizeof(size_t));
    buf.insert(buf.end(), min_value_.begin(), min_value_.end());
}

template<>
void ColumnTyped<std::string>::AppendMaxBytes(std::vector<char>& buf) const {
    size_t len = max_value_.size();
    const char* ptr = reinterpret_cast<const char*>(&len);
    buf.insert(buf.end(), ptr, ptr + sizeof(size_t));
    buf.insert(buf.end(), max_value_.begin(), max_value_.end());
}
