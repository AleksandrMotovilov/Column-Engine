#include "src/execution/sort_operators.h"

SortOperator::SortOperator(std::shared_ptr<Operator> next, std::string sort_column, bool descending) {
    next_ = std::move(next);
    sort_column_ = std::move(sort_column);
    descending_ = descending;
    done_ = false;
}

std::shared_ptr<Batch> SortOperator::Next() {
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
    size_t sort_column_index = schema->FindColumn(sort_column_);
    if (sort_column_index == schema->GetColumnsNumber()) {
        return merged;
    }

    Type sort_type = schema->GetType(sort_column_index);
    std::function<int(size_t, size_t)> compare_function = MakeColumnComparator(merged->GetColumn(sort_column_index), sort_type);

    std::vector<size_t> indices(merged->GetRowsNumber());
    std::iota(indices.begin(), indices.end(), 0);
    std::stable_sort(indices.begin(), indices.end(),
        [&](size_t a, size_t b) {
            int cmp = compare_function(a, b);
            if (descending_) {
                cmp = -cmp;
            }
            return cmp < 0;
        }
    );

    size_t columns_number = schema->GetColumnsNumber();
    std::vector<std::shared_ptr<Column>> columns;
    for (size_t i = 0; i < columns_number; i++) {
        columns.push_back(CopyRowsTyped(merged->GetColumn(i), schema->GetType(i), indices));
    }

    return std::make_shared<Batch>(merged->GetRowsNumber(), schema, std::move(columns));
}
