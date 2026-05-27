#include "src/execution/top_k_operators.h"

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

std::shared_ptr<Batch> TopKOperator::Next() {
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
        std::vector<Type> columns_types = merged->GetTypes();
        std::vector<std::string> columns_names = merged->GetNames();
        size_t k = std::min(limit_, merged->GetRowsNumber());
        std::vector<size_t> indices(merged->GetRowsNumber());
        std::iota(indices.begin(), indices.end(), 0);

        std::vector<std::pair<size_t, Type>> sort_keys;
        for (const auto& column_name : sort_columns_) {
            for (size_t i = 0; i < columns_names.size(); i++) {
                if (columns_names[i] == column_name) {
                    sort_keys.emplace_back(i, columns_types[i]);
                    break;
                }
            }
        }

        if (!sort_keys.empty()) {
            std::partial_sort(indices.begin(), indices.begin() + k, indices.end(),
                [&](size_t a, size_t b) -> bool {
                    for (const auto& key : sort_keys) {
                        int cmp = CompareValues(
                            merged->GetValue(a, key.first),
                            merged->GetValue(b, key.first),
                            key.second);
                        if (descending_) { cmp = -cmp; }
                        if (cmp < 0) { return true; }
                        if (cmp > 0) { return false; }
                    }
                    return false;
                });
        }

        columns_types.resize(merged->GetColumnsNumber());
        columns_names.resize(merged->GetColumnsNumber());
        std::shared_ptr<Batch> result = std::make_shared<Batch>(k, merged->GetColumnsNumber(), columns_types, columns_names);
        for (size_t i = 0; i < k; i++) {
            for (size_t j = 0; j < merged->GetColumnsNumber(); j++) {
                result->SetValue(i, j, merged->GetValue(indices[i], j));
            }
        }
        return result;
    }
    return nullptr;
}
