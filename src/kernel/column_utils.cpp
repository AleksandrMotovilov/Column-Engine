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
            const std::vector<int16_t>& raw = dynamic_cast<const ColumnTyped<int16_t>&>(*column).GetData();
            std::vector<int16_t> column_data;
            column_data.reserve(indices.size());
            for (size_t i : indices) {
                column_data.push_back(raw[i]);
            }
            return std::make_shared<ColumnTyped<int16_t>>(std::move(column_data));
        }
        case Type::Int32: {
            const std::vector<int32_t>& raw = dynamic_cast<const ColumnTyped<int32_t>&>(*column).GetData();
            std::vector<int32_t> column_data;
            column_data.reserve(indices.size());
            for (size_t i : indices) {
                column_data.push_back(raw[i]);
            }
            return std::make_shared<ColumnTyped<int32_t>>(std::move(column_data));
        }
        case Type::Int64: {
            const std::vector<int64_t>& raw = dynamic_cast<const ColumnTyped<int64_t>&>(*column).GetData();
            std::vector<int64_t> column_data;
            column_data.reserve(indices.size());
            for (size_t i : indices) {
                column_data.push_back(raw[i]);
            }
            return std::make_shared<ColumnTyped<int64_t>>(std::move(column_data));
        }
        case Type::Float: {
            const std::vector<float>& raw = dynamic_cast<const ColumnTyped<float>&>(*column).GetData();
            std::vector<float> column_data;
            column_data.reserve(indices.size());
            for (size_t i : indices) {
                column_data.push_back(raw[i]);
            }
            return std::make_shared<ColumnTyped<float>>(std::move(column_data));
        }
        case Type::Double: {
            const std::vector<double>& raw = dynamic_cast<const ColumnTyped<double>&>(*column).GetData();
            std::vector<double> column_data;
            column_data.reserve(indices.size());
            for (size_t i : indices) {
                column_data.push_back(raw[i]);
            }
            return std::make_shared<ColumnTyped<double>>(std::move(column_data));
        }
        case Type::Date: {
            const std::vector<Date>& raw = dynamic_cast<const ColumnTyped<Date>&>(*column).GetData();
            std::vector<Date> column_data;
            column_data.reserve(indices.size());
            for (size_t i : indices) {
                column_data.push_back(raw[i]);
            }
            return std::make_shared<ColumnTyped<Date>>(std::move(column_data));
        }
        case Type::Timestamp: {
            const std::vector<Timestamp>& raw = dynamic_cast<const ColumnTyped<Timestamp>&>(*column).GetData();
            std::vector<Timestamp> column_data;
            column_data.reserve(indices.size());
            for (size_t i : indices) {
                column_data.push_back(raw[i]);
            }
            return std::make_shared<ColumnTyped<Timestamp>>(std::move(column_data));
        }
        case Type::Char: {
            const std::vector<char>& raw = dynamic_cast<const ColumnTyped<char>&>(*column).GetData();
            std::vector<char> column_data;
            column_data.reserve(indices.size());
            for (size_t i : indices) {
                column_data.push_back(raw[i]);
            }
            return std::make_shared<ColumnTyped<char>>(std::move(column_data));
        }
        case Type::String: {
            const std::vector<std::string>& raw = dynamic_cast<const ColumnTyped<std::string>&>(*column).GetData();
            std::vector<std::string> column_data;
            column_data.reserve(indices.size());
            for (size_t i : indices) {
                column_data.push_back(raw[i]);
            }
            return std::make_shared<ColumnTyped<std::string>>(std::move(column_data));
        }
        default:
            throw std::runtime_error("Unsupported type :: CopyRowsTyped");
    }
}

std::shared_ptr<Column> MakeColumnFromStrings(Type column_type, const std::vector<std::string>& strs) {
    size_t rows_number = strs.size();
    switch (column_type) {
        case Type::Int16: {
            std::vector<int16_t> column_data;
            column_data.reserve(rows_number);
            for (const std::string& s : strs) {
                column_data.push_back(FromString<int16_t>(s));
            }
            return std::make_shared<ColumnTyped<int16_t>>(std::move(column_data));
        }
        case Type::Int32: {
            std::vector<int32_t> column_data;
            column_data.reserve(rows_number);
            for (const std::string& s : strs) {
                column_data.push_back(FromString<int32_t>(s));
            }
            return std::make_shared<ColumnTyped<int32_t>>(std::move(column_data));
        }
        case Type::Int64: {
            std::vector<int64_t> column_data;
            column_data.reserve(rows_number);
            for (const std::string& s : strs) {
                column_data.push_back(FromString<int64_t>(s));
            }
            return std::make_shared<ColumnTyped<int64_t>>(std::move(column_data));
        }
        case Type::Float: {
            std::vector<float> column_data;
            column_data.reserve(rows_number);
            for (const std::string& s : strs) {
                column_data.push_back(FromString<float>(s));
            }
            return std::make_shared<ColumnTyped<float>>(std::move(column_data));
        }
        case Type::Double: {
            std::vector<double> column_data;
            column_data.reserve(rows_number);
            for (const std::string& s : strs) {
                column_data.push_back(FromString<double>(s));
            }
            return std::make_shared<ColumnTyped<double>>(std::move(column_data));
        }
        case Type::Date: {
            std::vector<Date> column_data;
            column_data.reserve(rows_number);
            for (const std::string& s : strs) {
                column_data.push_back(FromString<Date>(s));
            }
            return std::make_shared<ColumnTyped<Date>>(std::move(column_data));
        }
        case Type::Timestamp: {
            std::vector<Timestamp> column_data;
            column_data.reserve(rows_number);
            for (const std::string& s : strs) {
                column_data.push_back(FromString<Timestamp>(s));
            }
            return std::make_shared<ColumnTyped<Timestamp>>(std::move(column_data));
        }
        case Type::Char: {
            std::vector<char> column_data;
            column_data.reserve(rows_number);
            for (const std::string& s : strs) {
                column_data.push_back(FromString<char>(s));
            }
            return std::make_shared<ColumnTyped<char>>(std::move(column_data));
        }
        case Type::String: {
            std::vector<std::string> column_data = strs;
            return std::make_shared<ColumnTyped<std::string>>(std::move(column_data));
        }
        default:
            throw std::runtime_error("Unsupported type :: MakeColumnFromStrings");
    }
}

std::function<int(size_t, size_t)> MakeColumnComparator(std::shared_ptr<Column> column, Type column_type) {
    switch (column_type) {
        case Type::Int16: {
            const std::vector<int16_t>& data = dynamic_cast<const ColumnTyped<int16_t>&>(*column).GetData();
            return [&data](size_t a, size_t b) {
                if (data[a] < data[b]) {
                    return -1;
                }
                if (data[a] > data[b]) {
                    return 1;
                }
                return 0;
            };
        }
        case Type::Int32: {
            const std::vector<int32_t>& data = dynamic_cast<const ColumnTyped<int32_t>&>(*column).GetData();
            return [&data](size_t a, size_t b) {
                if (data[a] < data[b]) {
                    return -1;
                }
                if (data[a] > data[b]) {
                    return 1;
                }
                return 0;
            };
        }
        case Type::Int64: {
            const std::vector<int64_t>& data = dynamic_cast<const ColumnTyped<int64_t>&>(*column).GetData();
            return [&data](size_t a, size_t b) {
                if (data[a] < data[b]) {
                    return -1;
                }
                if (data[a] > data[b]) {
                    return 1;
                }
                return 0;
            };
        }
        case Type::Float: {
            const std::vector<float>& data = dynamic_cast<const ColumnTyped<float>&>(*column).GetData();
            return [&data](size_t a, size_t b) {
                if (data[a] < data[b]) {
                    return -1;
                }
                if (data[a] > data[b]) {
                    return 1;
                }
                return 0;
            };
        }
        case Type::Double: {
            const std::vector<double>& data = dynamic_cast<const ColumnTyped<double>&>(*column).GetData();
            return [&data](size_t a, size_t b) {
                if (data[a] < data[b]) {
                    return -1;
                }
                if (data[a] > data[b]) {
                    return 1;
                }
                return 0;
            };
        }
        case Type::Date: {
            const std::vector<Date>& data = dynamic_cast<const ColumnTyped<Date>&>(*column).GetData();
            return [&data](size_t a, size_t b) {
                if (data[a] < data[b]) {
                    return -1;
                }
                if (data[a] > data[b]) {
                    return 1;
                }
                return 0;
            };
        }
        case Type::Timestamp: {
            const std::vector<Timestamp>& data = dynamic_cast<const ColumnTyped<Timestamp>&>(*column).GetData();
            return [&data](size_t a, size_t b) {
                if (data[a] < data[b]) {
                    return -1;
                }
                if (data[a] > data[b]) {
                    return 1;
                }
                return 0;
            };
        }
        case Type::Char: {
            const std::vector<char>& data = dynamic_cast<const ColumnTyped<char>&>(*column).GetData();
            return [&data](size_t a, size_t b) {
                if (data[a] < data[b]) {
                    return -1;
                }
                if (data[a] > data[b]) {
                    return 1;
                }
                return 0;
            };
        }
        case Type::String: {
            const std::vector<std::string>& data = dynamic_cast<const ColumnTyped<std::string>&>(*column).GetData();
            return [&data](size_t a, size_t b) {
                if (data[a] < data[b]) {
                    return -1;
                }
                if (data[a] > data[b]) {
                    return 1;
                }
                return 0;
            };
        }
        default:
            throw std::runtime_error("Unsupported type :: MakeColumnComparator");
    }
}

std::shared_ptr<Column> MakeColumnFromBytes(const std::vector<char>& buf, Type column_type, size_t rows_number) {
    switch (column_type) {
        case Type::Int16: {
            std::vector<int16_t> data(rows_number);
            std::memcpy(data.data(), buf.data(), rows_number * sizeof(int16_t));
            return std::make_shared<ColumnTyped<int16_t>>(std::move(data));
        }
        case Type::Int32: {
            std::vector<int32_t> data(rows_number);
            std::memcpy(data.data(), buf.data(), rows_number * sizeof(int32_t));
            return std::make_shared<ColumnTyped<int32_t>>(std::move(data));
        }
        case Type::Int64: {
            std::vector<int64_t> data(rows_number);
            std::memcpy(data.data(), buf.data(), rows_number * sizeof(int64_t));
            return std::make_shared<ColumnTyped<int64_t>>(std::move(data));
        }
        case Type::Float: {
            std::vector<float> data(rows_number);
            std::memcpy(data.data(), buf.data(), rows_number * sizeof(float));
            return std::make_shared<ColumnTyped<float>>(std::move(data));
        }
        case Type::Double: {
            std::vector<double> data(rows_number);
            std::memcpy(data.data(), buf.data(), rows_number * sizeof(double));
            return std::make_shared<ColumnTyped<double>>(std::move(data));
        }
        case Type::Date: {
            const int32_t* raw = reinterpret_cast<const int32_t*>(buf.data());
            std::vector<Date> data;
            data.reserve(rows_number);
            for (size_t i = 0; i < rows_number; i++) {
                data.emplace_back(raw[i]);
            }
            return std::make_shared<ColumnTyped<Date>>(std::move(data));
        }
        case Type::Timestamp: {
            const int64_t* raw = reinterpret_cast<const int64_t*>(buf.data());
            std::vector<Timestamp> data;
            data.reserve(rows_number);
            for (size_t i = 0; i < rows_number; i++) {
                data.emplace_back(raw[i]);
            }
            return std::make_shared<ColumnTyped<Timestamp>>(std::move(data));
        }
        case Type::Char: {
            std::vector<char> data(rows_number);
            std::memcpy(data.data(), buf.data(), rows_number * sizeof(char));
            return std::make_shared<ColumnTyped<char>>(std::move(data));
        }
        case Type::String: {
            std::vector<std::string> data;
            data.reserve(rows_number);
            size_t offset = 0;
            for (size_t i = 0; i < rows_number; i++) {
                size_t len;
                std::memcpy(&len, buf.data() + offset, sizeof(size_t));
                offset += sizeof(size_t);
                data.emplace_back(buf.data() + offset, len);
                offset += len;
            }
            return std::make_shared<ColumnTyped<std::string>>(std::move(data));
        }
        default:
            throw std::runtime_error("Unsupported type :: MakeColumnFromBytes");
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
