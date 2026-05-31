#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include "src/kernel/batch.h"
#include "src/kernel/column.h"

class ReaderCsv {
public:
    explicit ReaderCsv(const std::string& file_csv, const std::string& file_scheme);
    ~ReaderCsv();
    void ReadSchema();
    std::shared_ptr<Schema> GetSchema() const;
    std::shared_ptr<Batch> ReadBatch();

private:
    std::vector<std::vector<std::string>> ReadRowsStrings();

    std::ifstream fin_;
    std::string file_scheme_;
    std::shared_ptr<Schema> schema_;
    bool done_;
};

class WriterCsv {
public:
    explicit WriterCsv(const std::string& file_csv, const std::string& file_scheme);
    ~WriterCsv();
    void WriteBatch(std::shared_ptr<Batch> batch);
    void WriteSchema(std::shared_ptr<Schema> schema);
    void WriteFooter();

private:
    std::ofstream fout_csv_;
    std::string file_scheme_;
};
