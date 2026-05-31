#include "src/kernel/column.h"

template<>
ColumnTyped<Date>::ColumnTyped(std::vector<Date>&& data) {
    column_ = std::move(data);
    if (column_.empty()) {
        return;
    }
    Date min_val = column_[0];
    Date max_val = column_[0];
    __int128 sum = 0;
    for (const Date& val : column_) {
        if (val < min_val) {
            min_val = val;
        }
        if (val > max_val) {
            max_val = val;
        }
        sum += static_cast<__int128>(val.GetValue());
    }
    min_val_ = min_val;
    max_val_ = max_val;
    sum_ = sum;
}

template<>
ColumnTyped<Timestamp>::ColumnTyped(std::vector<Timestamp>&& data) {
    column_ = std::move(data);
    if (column_.empty()) {
        return;
    }
    Timestamp min_val = column_[0];
    Timestamp max_val = column_[0];
    __int128 sum = 0;
    for (const Timestamp& val : column_) {
        if (val < min_val) {
            min_val = val;
        }
        if (val > max_val) {
            max_val = val;
        }
        sum += static_cast<__int128>(val.GetValue());
    }
    min_val_ = min_val;
    max_val_ = max_val;
    sum_ = sum;
}
