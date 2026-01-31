#pragma once

#include "objects.h"
#include <string>
#include <memory>
#include <fstream>

class CLMNReader {
public:
    CLMNReader(const std::string& file, std::shared_ptr<Schema> schema);
    std::shared_ptr<Table> Read();
    ~CLMNReader();

private:
    std::ifstream fin_;
    std::shared_ptr<Schema> schema_;
};

class CSVWriter {
public:
    CSVWriter(const std::string& file, std::shared_ptr<Schema> schema);
    void Write(std::shared_ptr<Table> table);
    void WriteSchema(const std::string& file);
    ~CSVWriter();

private:
    std::ofstream fout_;
    std::shared_ptr<Schema> schema_;
};

void ConvertFromClmnToCsv(const std::string& file_clmn, const std::string& file_scv, const std::string& file_scheme);