#pragma once

#include <memory>
#include <string>
#include <vector>
#include "src/kernel/batch.h"
#include "src/kernel/column.h"

std::string GetStringValueAt(std::shared_ptr<Column> column, Type column_type, size_t index);

std::shared_ptr<Column> CopyRowsTyped(std::shared_ptr<Column> column, Type column_type, const std::vector<size_t>& indices);

std::shared_ptr<Column> MakeSingleValueColumn(Type column_type, const std::string& value);

std::shared_ptr<Column> MakeColumnFromStrings(Type column_type, const std::vector<std::string>& strs);

int CompareStringValues(const std::string& a, const std::string& b, Type column_type);

std::shared_ptr<Batch> MergeBatchesByRows(const std::vector<std::shared_ptr<Batch>>& batches);
