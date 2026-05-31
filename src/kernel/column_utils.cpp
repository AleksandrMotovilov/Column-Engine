#include "src/kernel/column_utils.h"

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
            std::vector<int16_t> result;
            result.reserve(indices.size());
            for (size_t i : indices) {
                result.push_back(column_data[i]);
            }
            return std::make_shared<ColumnTyped<int16_t>>(std::move(result));
        }
        case Type::Int32: {
            const std::vector<int32_t>& column_data = dynamic_cast<const ColumnTyped<int32_t>&>(*column).GetData();
            std::vector<int32_t> result;
            result.reserve(indices.size());
            for (size_t i : indices) {
                result.push_back(column_data[i]);
            }
            return std::make_shared<ColumnTyped<int32_t>>(std::move(result));
        }
        case Type::Int64: {
            const std::vector<int64_t>& column_data = dynamic_cast<const ColumnTyped<int64_t>&>(*column).GetData();
            std::vector<int64_t> result;
            result.reserve(indices.size());
            for (size_t i : indices) {
                result.push_back(column_data[i]);
            }
            return std::make_shared<ColumnTyped<int64_t>>(std::move(result));
        }
        case Type::Float: {
            const std::vector<float>& column_data = dynamic_cast<const ColumnTyped<float>&>(*column).GetData();
            std::vector<float> result;
            result.reserve(indices.size());
            for (size_t i : indices) {
                result.push_back(column_data[i]);
            }
            return std::make_shared<ColumnTyped<float>>(std::move(result));
        }
        case Type::Double: {
            const std::vector<double>& column_data = dynamic_cast<const ColumnTyped<double>&>(*column).GetData();
            std::vector<double> result;
            result.reserve(indices.size());
            for (size_t i : indices) {
                result.push_back(column_data[i]);
            }
            return std::make_shared<ColumnTyped<double>>(std::move(result));
        }
        case Type::Date: {
            const std::vector<Date>& column_data = dynamic_cast<const ColumnTyped<Date>&>(*column).GetData();
            std::vector<Date> result;
            result.reserve(indices.size());
            for (size_t i : indices) {
                result.push_back(column_data[i]);
            }
            return std::make_shared<ColumnTyped<Date>>(std::move(result));
        }
        case Type::Timestamp: {
            const std::vector<Timestamp>& column_data = dynamic_cast<const ColumnTyped<Timestamp>&>(*column).GetData();
            std::vector<Timestamp> result;
            result.reserve(indices.size());
            for (size_t i : indices) {
                result.push_back(column_data[i]);
            }
            return std::make_shared<ColumnTyped<Timestamp>>(std::move(result));
        }
        case Type::Char: {
            const std::vector<char>& column_data = dynamic_cast<const ColumnTyped<char>&>(*column).GetData();
            std::vector<char> result;
            result.reserve(indices.size());
            for (size_t i : indices) {
                result.push_back(column_data[i]);
            }
            return std::make_shared<ColumnTyped<char>>(std::move(result));
        }
        case Type::String: {
            const std::vector<std::string>& column_data = dynamic_cast<const ColumnTyped<std::string>&>(*column).GetData();
            std::vector<std::string> result;
            result.reserve(indices.size());
            for (size_t i : indices) {
                result.push_back(column_data[i]);
            }
            return std::make_shared<ColumnTyped<std::string>>(std::move(result));
        }
        default:
            throw std::runtime_error("Unsupported type :: CopyRowsTyped");
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
        default:
            throw std::runtime_error("Unsupported type :: MakeSingleValueColumn");
    }
}

std::shared_ptr<Column> MakeColumnFromStrings(Type column_type, const std::vector<std::string>& strs) {
    size_t rows_number = strs.size();
    switch (column_type) {
        case Type::Int16: {
            std::vector<int16_t> column;
            column.reserve(rows_number);
            for (const std::string& s : strs) {
                column.push_back(FromString<int16_t>(s));
            }
            return std::make_shared<ColumnTyped<int16_t>>(std::move(column));
        }
        case Type::Int32: {
            std::vector<int32_t> column;
            column.reserve(rows_number);
            for (const std::string& s : strs) {
                column.push_back(FromString<int32_t>(s));
            }
            return std::make_shared<ColumnTyped<int32_t>>(std::move(column));
        }
        case Type::Int64: {
            std::vector<int64_t> column;
            column.reserve(rows_number);
            for (const std::string& s : strs) {
                column.push_back(FromString<int64_t>(s));
            }
            return std::make_shared<ColumnTyped<int64_t>>(std::move(column));
        }
        case Type::Float: {
            std::vector<float> column;
            column.reserve(rows_number);
            for (const std::string& s : strs) {
                column.push_back(FromString<float>(s));
            }
            return std::make_shared<ColumnTyped<float>>(std::move(column));
        }
        case Type::Double: {
            std::vector<double> column;
            column.reserve(rows_number);
            for (const std::string& s : strs) {
                column.push_back(FromString<double>(s));
            }
            return std::make_shared<ColumnTyped<double>>(std::move(column));
        }
        case Type::Date: {
            std::vector<Date> column;
            column.reserve(rows_number);
            for (const std::string& s : strs) {
                column.push_back(FromString<Date>(s));
            }
            return std::make_shared<ColumnTyped<Date>>(std::move(column));
        }
        case Type::Timestamp: {
            std::vector<Timestamp> column;
            column.reserve(rows_number);
            for (const std::string& s : strs) {
                column.push_back(FromString<Timestamp>(s));
            }
            return std::make_shared<ColumnTyped<Timestamp>>(std::move(column));
        }
        case Type::Char: {
            std::vector<char> column;
            column.reserve(rows_number);
            for (const std::string& s : strs) {
                column.push_back(FromString<char>(s));
            }
            return std::make_shared<ColumnTyped<char>>(std::move(column));
        }
        case Type::String: {
            std::vector<std::string> column = strs;
            return std::make_shared<ColumnTyped<std::string>>(std::move(column));
        }
        default:
            throw std::runtime_error("Unsupported type :: MakeColumnFromStrings");
    }
}

int CompareStringValues(const std::string& a, const std::string& b, Type column_type) {
    switch (column_type) {
        case Type::Int16: {
            int16_t va = FromString<int16_t>(a), vb = FromString<int16_t>(b);
            if (va < vb) {
                return -1;
            }
            if (va > vb) {
                return 1;
            }
            return 0;
        }
        case Type::Int32: {
            int32_t va = FromString<int32_t>(a), vb = FromString<int32_t>(b);
            if (va < vb) {
                return -1;
            }
            if (va > vb) {
                return 1;
            }
            return 0;
        }
        case Type::Int64: {
            int64_t va = FromString<int64_t>(a), vb = FromString<int64_t>(b);
            if (va < vb) {
                return -1;
            }
            if (va > vb) {
                return 1;
            }
            return 0;
        }
        case Type::Float: {
            float va = FromString<float>(a), vb = FromString<float>(b);
            if (va < vb) {
                return -1;
            }
            if (va > vb) {
                return 1;
            }
            return 0;
        }
        case Type::Double: {
            double va = FromString<double>(a), vb = FromString<double>(b);
            if (va < vb) {
                return -1;
            }
            if (va > vb) {
                return 1;
            }
            return 0;
        }
        case Type::Date: {
            int32_t va = FromString<Date>(a).GetValue(), vb = FromString<Date>(b).GetValue();
            if (va < vb) {
                return -1;
            }
            if (va > vb) { return 1; }
            return 0;
        }
        case Type::Timestamp: {
            int64_t va = FromString<Timestamp>(a).GetValue(), vb = FromString<Timestamp>(b).GetValue();
            if (va < vb) {
                return -1;
            }
            if (va > vb) {
                return 1;
            }
            return 0;
        }
        case Type::Char: {
            char va = FromString<char>(a), vb = FromString<char>(b);
            if (va < vb) {
                return -1;
            }
            if (va > vb) {
                return 1;
            }
            return 0;
        }
        case Type::String: {
            if (a < b) {
                return -1;
            }
            if (a > b) {
                return 1;
            }
            return 0;
        }
        default:
            throw std::runtime_error("Unsupported type :: CompareStringValues");
    }
}

std::shared_ptr<Batch> MergeBatchesByRows(const std::vector<std::shared_ptr<Batch>>& batches) {
    if (batches.empty()) {
        return nullptr;
    }
    std::shared_ptr<Schema> schema = batches[0]->GetSchema();
    size_t total_rows = 0;
    for (const std::shared_ptr<Batch>& batch : batches) {
        total_rows += batch->GetRowsNumber();
    }
    size_t columns_number = schema->GetColumnsNumber();
    std::vector<std::shared_ptr<Column>> merged_columns;
    merged_columns.reserve(columns_number);
    for (size_t column_index = 0; column_index < columns_number; column_index++) {
        switch (schema->GetType(column_index)) {
            case Type::Int16: {
                std::vector<int16_t> column_data;
                column_data.reserve(total_rows);
                for (const std::shared_ptr<Batch>& batch : batches) {
                    const std::vector<int16_t>& src = dynamic_cast<const ColumnTyped<int16_t>&>(*batch->GetColumn(column_index)).GetData();
                    column_data.insert(column_data.end(), src.begin(), src.end());
                }
                merged_columns.push_back(std::make_shared<ColumnTyped<int16_t>>(std::move(column_data)));
                break;
            }
            case Type::Int32: {
                std::vector<int32_t> column_data;
                column_data.reserve(total_rows);
                for (const std::shared_ptr<Batch>& batch : batches) {
                    const std::vector<int32_t>& src = dynamic_cast<const ColumnTyped<int32_t>&>(*batch->GetColumn(column_index)).GetData();
                    column_data.insert(column_data.end(), src.begin(), src.end());
                }
                merged_columns.push_back(std::make_shared<ColumnTyped<int32_t>>(std::move(column_data)));
                break;
            }
            case Type::Int64: {
                std::vector<int64_t> column_data;
                column_data.reserve(total_rows);
                for (const std::shared_ptr<Batch>& batch : batches) {
                    const std::vector<int64_t>& src = dynamic_cast<const ColumnTyped<int64_t>&>(*batch->GetColumn(column_index)).GetData();
                    column_data.insert(column_data.end(), src.begin(), src.end());
                }
                merged_columns.push_back(std::make_shared<ColumnTyped<int64_t>>(std::move(column_data)));
                break;
            }
            case Type::Float: {
                std::vector<float> column_data;
                column_data.reserve(total_rows);
                for (const std::shared_ptr<Batch>& batch : batches) {
                    const std::vector<float>& src = dynamic_cast<const ColumnTyped<float>&>(*batch->GetColumn(column_index)).GetData();
                    column_data.insert(column_data.end(), src.begin(), src.end());
                }
                merged_columns.push_back(std::make_shared<ColumnTyped<float>>(std::move(column_data)));
                break;
            }
            case Type::Double: {
                std::vector<double> column_data;
                column_data.reserve(total_rows);
                for (const std::shared_ptr<Batch>& batch : batches) {
                    const std::vector<double>& src = dynamic_cast<const ColumnTyped<double>&>(*batch->GetColumn(column_index)).GetData();
                    column_data.insert(column_data.end(), src.begin(), src.end());
                }
                merged_columns.push_back(std::make_shared<ColumnTyped<double>>(std::move(column_data)));
                break;
            }
            case Type::Date: {
                std::vector<Date> column_data;
                column_data.reserve(total_rows);
                for (const std::shared_ptr<Batch>& batch : batches) {
                    const std::vector<Date>& src = dynamic_cast<const ColumnTyped<Date>&>(*batch->GetColumn(column_index)).GetData();
                    column_data.insert(column_data.end(), src.begin(), src.end());
                }
                merged_columns.push_back(std::make_shared<ColumnTyped<Date>>(std::move(column_data)));
                break;
            }
            case Type::Timestamp: {
                std::vector<Timestamp> column_data;
                column_data.reserve(total_rows);
                for (const std::shared_ptr<Batch>& batch : batches) {
                    const std::vector<Timestamp>& src = dynamic_cast<const ColumnTyped<Timestamp>&>(*batch->GetColumn(column_index)).GetData();
                    column_data.insert(column_data.end(), src.begin(), src.end());
                }
                merged_columns.push_back(std::make_shared<ColumnTyped<Timestamp>>(std::move(column_data)));
                break;
            }
            case Type::Char: {
                std::vector<char> column_data;
                column_data.reserve(total_rows);
                for (const std::shared_ptr<Batch>& batch : batches) {
                    const std::vector<char>& src = dynamic_cast<const ColumnTyped<char>&>(*batch->GetColumn(column_index)).GetData();
                    column_data.insert(column_data.end(), src.begin(), src.end());
                }
                merged_columns.push_back(std::make_shared<ColumnTyped<char>>(std::move(column_data)));
                break;
            }
            case Type::String: {
                std::vector<std::string> column_data;
                column_data.reserve(total_rows);
                for (const std::shared_ptr<Batch>& batch : batches) {
                    const std::vector<std::string>& src = dynamic_cast<const ColumnTyped<std::string>&>(*batch->GetColumn(column_index)).GetData();
                    column_data.insert(column_data.end(), src.begin(), src.end());
                }
                merged_columns.push_back(std::make_shared<ColumnTyped<std::string>>(std::move(column_data)));
                break;
            }
            default:
                throw std::runtime_error("Unsupported type :: MergeBatchesByRows");
        }
    }
    return std::make_shared<Batch>(total_rows, schema, std::move(merged_columns));
}
