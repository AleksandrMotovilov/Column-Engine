#include "src/execution/sort_operators.h"
#include "src/column.h"

SortOperator::SortOperator(std::shared_ptr<Operator> next, std::string sort_column, bool descending) {
    next_ = std::move(next);
    sort_column_ = std::move(sort_column);
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

std::shared_ptr<Batch> SortOperator::Next() {
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

    std::vector<std::string> columns_names = merged->GetNames();
    std::vector<Type> columns_types = merged->GetTypes();

    size_t sort_column_index = columns_names.size();
    for (size_t i = 0; i < columns_names.size(); i++) {
        if (columns_names[i] == sort_column_) {
            sort_column_index = i;
            break;
        }
    }

    if (sort_column_index == columns_names.size()) {
        return merged;
    }

    Type sort_type = columns_types[sort_column_index];
    std::vector<std::string> sort_column_strings(merged->GetRowsNumber());
    for (size_t i = 0; i < merged->GetRowsNumber(); i++) {
        sort_column_strings[i] = GetStringValueAt(merged->GetColumn(sort_column_index), sort_type, i);
    }

    std::vector<size_t> indices(merged->GetRowsNumber());
    std::iota(indices.begin(), indices.end(), 0);
    std::stable_sort(indices.begin(), indices.end(),
        [&](size_t a, size_t b) {
            int cmp = CompareValues(sort_column_strings[a], sort_column_strings[b], sort_type);
            if (descending_) {
                cmp = -cmp;
            }
            return cmp < 0;
        }
    );

    size_t columns_number = merged->GetColumnsNumber();
    std::vector<std::shared_ptr<Column>> columns;
    for (size_t i = 0; i < columns_number; i++) {
        columns.push_back(CopyRowsTyped(merged->GetColumn(i), columns_types[i], indices));
    }

    return std::make_shared<Batch>(merged->GetRowsNumber(), std::move(columns_names), std::move(columns_types), std::move(columns));
}
