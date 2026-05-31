#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "src/kernel/column.h"
#include "src/kernel/schema.h"

inline size_t kColumnBatchSize = 2;
inline size_t kRowBatchSize = 2;

void SetBatchSize(size_t column_batch_size, size_t row_batch_size);

class Batch {
public:
    Batch(size_t rows_number, std::shared_ptr<Schema> schema, std::vector<std::shared_ptr<Column>> columns);
    size_t GetColumnsNumber() const;
    size_t GetRowsNumber() const;
    const std::string& GetName(size_t index) const;
    Type GetType(size_t index) const;
    size_t FindColumn(const std::string& name) const;
    std::shared_ptr<Column> GetColumn(size_t index) const;
    std::shared_ptr<Schema> GetSchema() const;
    std::vector<std::shared_ptr<Column>> MoveColumns();

private:
    size_t rows_number_;
    std::shared_ptr<Schema> schema_;
    std::vector<std::shared_ptr<Column>> columns_;
};
