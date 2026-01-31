#pragma once

#include "objects.h"
#include <string>
#include <memory>
#include <fstream>

class CSVReader {
public:
    CSVReader(const std::string& file, std::shared_ptr<Schema> schema);
    std::shared_ptr<Table> Read();
    ~CSVReader();

private:
    std::ifstream fin_;
    std::shared_ptr<Schema> schema_;
};

class CLMNWriter {
public:
    CLMNWriter(const std::string& file, std::shared_ptr<Schema> schema);
    void Write(std::shared_ptr<Table> table);
    void WriteSchema();
    ~CLMNWriter();

private:
    std::ofstream fout_;
    std::shared_ptr<Schema> schema_; 
};

void ConvertFromCsvToClmn(const std::string& file_csv, const std::string& file_scheme, const std::string& file_clmn);