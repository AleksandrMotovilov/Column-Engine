#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "src/column.h"

inline size_t kColumnBatchSize = 2;
inline size_t kRowBatchSize = 2;

void SetBatchSize(size_t column_batch_size, size_t row_batch_size);

class Batch {
public:
    Batch(size_t rows_number, std::vector<std::string> names, std::vector<Type> types, std::vector<std::shared_ptr<Column>> columns);
    size_t GetColumnsNumber() const;
    size_t GetRowsNumber() const;
    std::vector<Type> GetTypes() const;
    std::vector<std::string> GetNames() const;
    size_t FindColumn(const std::string& name) const;
    std::shared_ptr<Column> GetColumn(size_t index) const;
    std::vector<std::shared_ptr<Column>> MoveColumns();

private:
    size_t rows_number_;
    std::vector<std::shared_ptr<Column>> columns_;
    std::vector<std::string> names_;
    std::vector<Type> types_;
};
