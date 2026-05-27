#pragma once

#include <algorithm>
#include <fstream>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>
#include "src/execution/operators.h"
#include "src/objects.h"

class ScanOperator : public Operator {
public:
    ScanOperator(const std::string& file_clmn, std::vector<std::string> needed_columns);
    std::shared_ptr<Batch> Next() override;

private:
    std::ifstream fin_;
    std::vector<std::string> needed_columns_;
    std::vector<std::string> available_columns_;
    std::map<std::string, size_t> columns_data_pos_;
    std::map<std::string, Type> columns_types_;
    size_t rows_number_;
    size_t columns_number_;
    size_t columns_returned_;
};

