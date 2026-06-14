#include "src/execution/top_k_operators.h"

std::shared_ptr<Batch> TopKFromBatch(std::shared_ptr<Batch> batch, size_t k, const std::vector<std::pair<size_t, Type>>& sort_keys, bool descending) {
    size_t rows_number = batch->GetRowsNumber();
    size_t actual_k = std::min(k, rows_number);
    std::vector<size_t> indices(rows_number);
    std::iota(indices.begin(), indices.end(), 0);
    if (!sort_keys.empty()) {
        std::vector<std::vector<std::string>> sort_columns_strings(sort_keys.size());
        for (size_t i = 0; i < sort_keys.size(); i++) {
            sort_columns_strings[i].resize(rows_number);
            for (size_t j = 0; j < rows_number; j++) {
                sort_columns_strings[i][j] = GetStringValueAt(batch->GetColumn(sort_keys[i].first), sort_keys[i].second, j);
            }
        }
        std::partial_sort(indices.begin(), indices.begin() + actual_k, indices.end(),
            [&](size_t a, size_t b) -> bool {
                for (size_t i = 0; i < sort_keys.size(); i++) {
                    int cmp = CompareStringValues(sort_columns_strings[i][a], sort_columns_strings[i][b], sort_keys[i].second);
                    if (descending) {
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
    std::shared_ptr<Schema> schema = batch->GetSchema();
    std::vector<size_t> top_k_indices(indices.begin(), indices.begin() + actual_k);
    std::vector<std::shared_ptr<Column>> columns;
    for (size_t i = 0; i < schema->GetColumnsNumber(); i++) {
        columns.push_back(CopyRowsTyped(batch->GetColumn(i), schema->GetType(i), top_k_indices));
    }
    return std::make_shared<Batch>(actual_k, schema, std::move(columns));
}

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

    std::shared_ptr<Batch> first = next_->Next();
    if (!first) {
        return nullptr;
    }

    std::shared_ptr<Schema> schema = first->GetSchema();
    std::vector<std::pair<size_t, Type>> sort_keys;
    for (const std::string& column_name : sort_columns_) {
        size_t index = schema->FindColumn(column_name);
        if (index < schema->GetColumnsNumber()) {
            sort_keys.emplace_back(index, schema->GetType(index));
        }
    }

    std::shared_ptr<Batch> running = TopKFromBatch(first, limit_, sort_keys, descending_);
    while (std::shared_ptr<Batch> batch = next_->Next()) {
        std::shared_ptr<Batch> top_k_batch = TopKFromBatch(batch, limit_, sort_keys, descending_);
        std::shared_ptr<Batch> merged = MergeBatchesByRows({running, top_k_batch});
        running = TopKFromBatch(merged, limit_, sort_keys, descending_);
    }
    return running;
}
