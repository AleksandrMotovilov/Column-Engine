#include "src/execution/top_k_operators.h"
#include "src/kernel/column_utils.h"

TopKOperator::TopKOperator(std::shared_ptr<Operator> next, std::vector<std::string> sort_columns, size_t limit, bool descending) {
    next_ = std::move(next);
    sort_columns_ = std::move(sort_columns);
    limit_ = limit;
    descending_ = descending;
    done_ = false;
}

std::shared_ptr<Batch> TopKOperator::Next() {
    if (done_) {
        return nullptr;
    }
    done_ = true;

    std::vector<std::shared_ptr<Batch>> all_batches;
    while (std::shared_ptr<Batch> batch = next_->Next()) {
        all_batches.push_back(batch);
    }

    if (all_batches.empty()) {
        return nullptr;
    }

    std::shared_ptr<Batch> merged = MergeBatchesByRows(all_batches);
    if (merged == nullptr) {
        return nullptr;
    }

    std::shared_ptr<Schema> schema = merged->GetSchema();
    size_t k = std::min(limit_, merged->GetRowsNumber());
    std::vector<size_t> indices(merged->GetRowsNumber());
    std::iota(indices.begin(), indices.end(), 0);

    std::vector<std::pair<size_t, Type>> sort_keys;
    for (const std::string& column_name : sort_columns_) {
        size_t idx = schema->FindColumn(column_name);
        if (idx < schema->GetColumnsNumber()) {
            sort_keys.emplace_back(idx, schema->GetType(idx));
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
                    int cmp = CompareStringValues(sort_column_strings[i][a], sort_column_strings[i][b], sort_keys[i].second);
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
    size_t columns_number = schema->GetColumnsNumber();
    std::vector<std::shared_ptr<Column>> columns;
    for (size_t i = 0; i < columns_number; i++) {
        columns.push_back(CopyRowsTyped(merged->GetColumn(i), schema->GetType(i), top_k_indices));
    }

    return std::make_shared<Batch>(k, schema, std::move(columns));
}
