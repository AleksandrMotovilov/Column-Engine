#include "src/execution/sort_operators.h"

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
            int32_t va = FromString<Date>(a).days, vb = FromString<Date>(b).days;
            if (va < vb) {
                return -1;
            }
            if (va > vb) {
                return 1;
            }
            return 0;
        }
        case Type::Timestamp: {
            int64_t va = FromString<Timestamp>(a).seconds, vb = FromString<Timestamp>(b).seconds;
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
    std::shared_ptr<Batch> merged;
    while (std::shared_ptr<Batch> batch = next_->Next()) {
    if (merged == nullptr) {
            merged = batch;
            continue;
        }
    
        size_t total_columns = merged->GetColumnsNumber() + batch->GetColumnsNumber();
    
        std::vector<Type> columns_types = merged->GetTypes();
        std::vector<Type> batch_types = batch->GetTypes();
        columns_types.insert(columns_types.end(), batch_types.begin(), batch_types.end());
    
        std::vector<std::string> columns_names = merged->GetNames();
        std::vector<std::string> batch_names = batch->GetNames();
        columns_names.insert(columns_names.end(), batch_names.begin(), batch_names.end());
    
        std::shared_ptr<Batch> combined = std::make_shared<Batch>(merged->GetRowsNumber(), total_columns, columns_types, columns_names);
        for (size_t i = 0; i < merged->GetRowsNumber(); i++) {
            for (size_t j = 0; j < merged->GetColumnsNumber(); j++) {
                combined->SetValue(i, j, merged->GetValue(i, j));
            }
            for (size_t j = 0; j < batch->GetColumnsNumber(); j++) {
                combined->SetValue(i, merged->GetColumnsNumber() + j, batch->GetValue(i, j));
            }
        }
        merged = combined;
    }
    if (merged != nullptr) {
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
        } else {
            Type sort_type = columns_types[sort_column_index];
            std::vector<size_t> indices(merged->GetRowsNumber());
            std::iota(indices.begin(), indices.end(), 0);
            std::stable_sort(indices.begin(), indices.end(),
            [&](size_t a, size_t b) {
                int cmp = CompareValues(
                    merged->GetValue(a, sort_column_index),
                    merged->GetValue(b, sort_column_index),
                    sort_type);
                if (descending_) {
                    cmp = -cmp;
                }
                return cmp < 0;
            });
            std::shared_ptr<Batch> result = std::make_shared<Batch>(merged->GetRowsNumber(), merged->GetColumnsNumber(), columns_types, columns_names);
            for (size_t i = 0; i < merged->GetRowsNumber(); i++) {
                for (size_t j = 0; j < merged->GetColumnsNumber(); j++) {
                    result->SetValue(i, j, merged->GetValue(indices[i], j));
                }
            }
            return result;
        }
    }
    return nullptr;
}
