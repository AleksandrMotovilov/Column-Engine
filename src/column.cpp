#include "src/column.h"

template<>
ColumnTyped<Date>::ColumnTyped(std::vector<Date>&& data) {
    column_ = std::move(data);
    if (column_.empty()) {
        return;
    }
    std::unordered_set<int32_t> distinct;
    Date min_val = column_[0];
    Date max_val = column_[0];
    __int128 sum = 0;
    for (const Date& val : column_) {
        distinct.insert(val.GetValue());
        if (val < min_val) {
            min_val = val;
        }
        if (val > max_val) {
            max_val = val;
        }
        sum += static_cast<__int128>(val.GetValue());
    }
    count_distinct_ = static_cast<int64_t>(distinct.size());
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
    std::unordered_set<int64_t> distinct;
    Timestamp min_val = column_[0];
    Timestamp max_val = column_[0];
    __int128 sum = 0;
    for (const Timestamp& val : column_) {
        distinct.insert(val.GetValue());
        if (val < min_val) {
            min_val = val;
        }
        if (val > max_val) {
            max_val = val;
        }
        sum += static_cast<__int128>(val.GetValue());
    }
    count_distinct_ = static_cast<int64_t>(distinct.size());
    min_val_ = min_val;
    max_val_ = max_val;
    sum_ = sum;
}

std::string GetStringValueAt(std::shared_ptr<Column> column, Type column_type, size_t index) {
    switch (column_type) {
        case Type::Int16:
            return ToString<int16_t>(dynamic_cast<const ColumnTyped<int16_t>&>(*column).GetData()[index]);
        case Type::Int32:
            return ToString<int32_t>(dynamic_cast<const ColumnTyped<int32_t>&>(*column).GetData()[index]);
        case Type::Int64:
            return ToString<int64_t>(dynamic_cast<const ColumnTyped<int64_t>&>(*column).GetData()[index]);
        case Type::Float:
            return ToString<float>(dynamic_cast<const ColumnTyped<float>&>(*column).GetData()[index]);
        case Type::Double:
            return ToString<double>(dynamic_cast<const ColumnTyped<double>&>(*column).GetData()[index]);
        case Type::Date:
            return ToString<Date>(dynamic_cast<const ColumnTyped<Date>&>(*column).GetData()[index]);
        case Type::Timestamp:
            return ToString<Timestamp>(dynamic_cast<const ColumnTyped<Timestamp>&>(*column).GetData()[index]);
        case Type::Char:
            return ToString<char>(dynamic_cast<const ColumnTyped<char>&>(*column).GetData()[index]);
        case Type::String:
            return dynamic_cast<const ColumnTyped<std::string>&>(*column).GetData()[index];
        default:
            throw std::runtime_error("Unsupported type :: GetStringValueAt");
    }
}

std::shared_ptr<Column> CopyRowsTyped(std::shared_ptr<Column> column, Type column_type, const std::vector<size_t>& indices) {
    switch (column_type) {
        case Type::Int16: {
            const std::vector<int16_t>& column_data = dynamic_cast<const ColumnTyped<int16_t>&>(*column).GetData();
            std::vector<int16_t> new_column_data;
            new_column_data.reserve(indices.size());
            for (size_t i : indices) {
                new_column_data.push_back(column_data[i]);
            }
            return std::make_shared<ColumnTyped<int16_t>>(std::move(new_column_data));
        }
        case Type::Int32: {
            const std::vector<int32_t>& column_data = dynamic_cast<const ColumnTyped<int32_t>&>(*column).GetData();
            std::vector<int32_t> new_column_data;
            new_column_data.reserve(indices.size());
            for (size_t i : indices) {
                new_column_data.push_back(column_data[i]);
            }
            return std::make_shared<ColumnTyped<int32_t>>(std::move(new_column_data));
        }
        case Type::Int64: {
            const std::vector<int64_t>& column_data = dynamic_cast<const ColumnTyped<int64_t>&>(*column).GetData();
            std::vector<int64_t> new_column_data;
            new_column_data.reserve(indices.size());
            for (size_t i : indices) {
                new_column_data.push_back(column_data[i]);
            }
            return std::make_shared<ColumnTyped<int64_t>>(std::move(new_column_data));
        }
        case Type::Float: {
            const std::vector<float>& column_data = dynamic_cast<const ColumnTyped<float>&>(*column).GetData();
            std::vector<float> new_column_data;
            new_column_data.reserve(indices.size());
            for (size_t i : indices) {
                new_column_data.push_back(column_data[i]);
            }
            return std::make_shared<ColumnTyped<float>>(std::move(new_column_data));
        }
        case Type::Double: {
            const std::vector<double>& column_data = dynamic_cast<const ColumnTyped<double>&>(*column).GetData();
            std::vector<double> new_column_data;
            new_column_data.reserve(indices.size());
            for (size_t i : indices) {
                new_column_data.push_back(column_data[i]);
            }
            return std::make_shared<ColumnTyped<double>>(std::move(new_column_data));
        }
        case Type::Date: {
            const std::vector<Date>& column_data = dynamic_cast<const ColumnTyped<Date>&>(*column).GetData();
            std::vector<Date> new_column_data;
            new_column_data.reserve(indices.size());
            for (size_t i : indices) {
                new_column_data.push_back(column_data[i]);
            }
            return std::make_shared<ColumnTyped<Date>>(std::move(new_column_data));
        }
        case Type::Timestamp: {
            const std::vector<Timestamp>& column_data = dynamic_cast<const ColumnTyped<Timestamp>&>(*column).GetData();
            std::vector<Timestamp> new_column_data;
            new_column_data.reserve(indices.size());
            for (size_t i : indices) {
                new_column_data.push_back(column_data[i]);
            }
            return std::make_shared<ColumnTyped<Timestamp>>(std::move(new_column_data));
        }
        case Type::Char: {
            const std::vector<char>& column_data = dynamic_cast<const ColumnTyped<char>&>(*column).GetData();
            std::vector<char> new_column_data;
            new_column_data.reserve(indices.size());
            for (size_t i : indices) {
                new_column_data.push_back(column_data[i]);
            }
            return std::make_shared<ColumnTyped<char>>(std::move(new_column_data));
        }
        case Type::String: {
            const std::vector<std::string>& column_data = dynamic_cast<const ColumnTyped<std::string>&>(*column).GetData();
            std::vector<std::string> new_column_data;
            new_column_data.reserve(indices.size());
            for (size_t i : indices) {
                new_column_data.push_back(column_data[i]);
            }
            return std::make_shared<ColumnTyped<std::string>>(std::move(new_column_data));
        }
        default: {
            throw std::runtime_error("Unsupported type :: CopyRowsTyped");
        }
    }
}

std::shared_ptr<Column> MakeSingleValueColumn(Type column_type, const std::string& value) {
    switch (column_type) {
        case Type::Int16: {
            std::vector<int16_t> v = {FromString<int16_t>(value)};
            return std::make_shared<ColumnTyped<int16_t>>(std::move(v));
        }
        case Type::Int32: {
            std::vector<int32_t> v = {FromString<int32_t>(value)};
            return std::make_shared<ColumnTyped<int32_t>>(std::move(v));
        }
        case Type::Int64: {
            std::vector<int64_t> v = {FromString<int64_t>(value)};
            return std::make_shared<ColumnTyped<int64_t>>(std::move(v));
        }
        case Type::Float: {
            std::vector<float> v = {FromString<float>(value)};
            return std::make_shared<ColumnTyped<float>>(std::move(v));
        }
        case Type::Double: {
            std::vector<double> v = {FromString<double>(value)};
            return std::make_shared<ColumnTyped<double>>(std::move(v));
        }
        case Type::Date: {
            std::vector<Date> v = {FromString<Date>(value)};
            return std::make_shared<ColumnTyped<Date>>(std::move(v));
        }
        case Type::Timestamp: {
            std::vector<Timestamp> v = {FromString<Timestamp>(value)};
            return std::make_shared<ColumnTyped<Timestamp>>(std::move(v));
        }
        case Type::Char: {
            std::vector<char> v = {FromString<char>(value)};
            return std::make_shared<ColumnTyped<char>>(std::move(v));
        }
        case Type::String: {
            std::vector<std::string> v = {value};
            return std::make_shared<ColumnTyped<std::string>>(std::move(v));
        }
        default: {
            std::vector<std::string> v = {value};
            return std::make_shared<ColumnTyped<std::string>>(std::move(v));
        }
    }
}

std::shared_ptr<Column> MakeColumnFromStrings(Type column_type, const std::vector<std::string>& strs) {
    switch (column_type) {
        case Type::Int16: {
            std::vector<int16_t> v;
            v.reserve(strs.size());
            for (const std::string& s : strs) {
                v.push_back(FromString<int16_t>(s));
            }
            return std::make_shared<ColumnTyped<int16_t>>(std::move(v));
        }
        case Type::Int32: {
            std::vector<int32_t> v;
            v.reserve(strs.size());
            for (const std::string& s : strs) {
                v.push_back(FromString<int32_t>(s));
            }
            return std::make_shared<ColumnTyped<int32_t>>(std::move(v));
        }
        case Type::Int64: {
            std::vector<int64_t> v;
            v.reserve(strs.size());
            for (const std::string& s : strs) {
                v.push_back(FromString<int64_t>(s));
            }
            return std::make_shared<ColumnTyped<int64_t>>(std::move(v));
        }
        case Type::Float: {
            std::vector<float> v;
            v.reserve(strs.size());
            for (const std::string& s : strs) {
                v.push_back(FromString<float>(s));
            }
            return std::make_shared<ColumnTyped<float>>(std::move(v));
        }
        case Type::Double: {
            std::vector<double> v;
            v.reserve(strs.size());
            for (const std::string& s : strs) {
                v.push_back(FromString<double>(s));
            }
            return std::make_shared<ColumnTyped<double>>(std::move(v));
        }
        case Type::Date: {
            std::vector<Date> v;
            v.reserve(strs.size());
            for (const std::string& s : strs) {
                v.push_back(FromString<Date>(s));
            }
            return std::make_shared<ColumnTyped<Date>>(std::move(v));
        }
        case Type::Timestamp: {
            std::vector<Timestamp> v;
            v.reserve(strs.size());
            for (const std::string& s : strs) {
                v.push_back(FromString<Timestamp>(s));
            }
            return std::make_shared<ColumnTyped<Timestamp>>(std::move(v));
        }
        case Type::Char: {
            std::vector<char> v;
            v.reserve(strs.size());
            for (const std::string& s : strs) {
                v.push_back(FromString<char>(s));
            }
            return std::make_shared<ColumnTyped<char>>(std::move(v));
        }
        default: {
            std::vector<std::string> v = strs;
            return std::make_shared<ColumnTyped<std::string>>(std::move(v));
        }
    }
}
