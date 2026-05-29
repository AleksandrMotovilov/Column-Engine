#include "src/execution/top_k_operators.h"
#include "src/column.h"

TopKOperator::TopKOperator(std::shared_ptr<Operator> next, std::vector<std::string> sort_columns, size_t limit, bool descending) {
    next_ = std::move(next);
    sort_columns_ = std::move(sort_columns);
    limit_ = limit;
    descending_ = descending;
    done_ = false;
}

static int CompareValues(const std::string& a, const std::string& b, Type type) {
    switch (type) {
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
            if (va > vb) {
                return 1;
            }
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
        default: {
            if (a < b) {
                return -1;
            }
            if (a > b) {
                return 1;
            }
            return 0;
        }
    }
}

std::shared_ptr<Batch> TopKOperator::Next() {
    if (done_) {
        return nullptr;
    }
    done_ = true;

    std::vector<std::shared_ptr<Column>> all_columns;
    std::vector<std::string> all_names;
    std::vector<Type> all_types;
    size_t rows = 0;

    while (std::shared_ptr<Batch> batch = next_->Next()) {
        if (rows == 0) {
            rows = batch->GetRowsNumber();
        }
        std::vector<std::string> names = batch->GetNames();
        std::vector<Type> types = batch->GetTypes();
        all_names.insert(all_names.end(), names.begin(), names.end());
        all_types.insert(all_types.end(), types.begin(), types.end());
        std::vector<std::shared_ptr<Column>> columns = batch->MoveColumns();
        for (std::shared_ptr<Column>& column : columns) {
            all_columns.push_back(std::move(column));
        }
    }

    if (all_names.empty()) {
        return nullptr;
    }

    std::shared_ptr<Batch> merged = std::make_shared<Batch>(rows, std::move(all_names), std::move(all_types), std::move(all_columns));

    std::vector<Type> columns_types = merged->GetTypes();
    std::vector<std::string> columns_names = merged->GetNames();
    size_t k = std::min(limit_, merged->GetRowsNumber());
    std::vector<size_t> indices(merged->GetRowsNumber());
    std::iota(indices.begin(), indices.end(), 0);

    std::vector<std::pair<size_t, Type>> sort_keys;
    for (const std::string& column_name : sort_columns_) {
        for (size_t i = 0; i < columns_names.size(); i++) {
            if (columns_names[i] == column_name) {
                sort_keys.emplace_back(i, columns_types[i]);
                break;
            }
        }
    }

    if (!sort_keys.empty()) {
        std::vector<std::vector<std::string>> sort_column_strings(sort_keys.size());
        for (size_t i = 0; i < sort_keys.size(); i++) {
            size_t column_index = sort_keys[i].first;
            Type column_type = sort_keys[i].second;
            sort_column_strings[i].resize(merged->GetRowsNumber());
            for (size_t j = 0; j < merged->GetRowsNumber(); j++) {
                sort_column_strings[i][j] = GetStringValueAt(merged->GetColumn(column_index), column_type, j);
            }
        }
        std::partial_sort(indices.begin(), indices.begin() + k, indices.end(),
            [&](size_t a, size_t b) -> bool {
                for (size_t i = 0; i < sort_keys.size(); i++) {
                    int cmp = CompareValues(sort_column_strings[i][a], sort_column_strings[i][b], sort_keys[i].second);
                    if (descending_) {
                        cmp = -cmp;
                    }
                    if (cmp < 0) {
                        return true;
                    }
                    if (cmp > 0) {
                        return false;
                    }
                }
                return false;
            });
    }

    std::vector<size_t> top_k_indices(indices.begin(), indices.begin() + k);
    std::vector<std::shared_ptr<Column>> columns;
    size_t columns_number = merged->GetColumnsNumber();
    for (size_t i = 0; i < columns_number; i++) {
        columns.push_back(CopyRowsTyped(merged->GetColumn(i), columns_types[i], top_k_indices));
    }

    return std::make_shared<Batch>(k, std::move(columns_names), std::move(columns_types), std::move(columns));
}
