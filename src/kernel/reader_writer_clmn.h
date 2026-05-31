#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include "src/kernel/batch.h"

class ReaderClmn {
public:
    explicit ReaderClmn(const std::string& file_clmn);
    ~ReaderClmn();
    void ReadSchema();
    std::shared_ptr<Schema> GetSchema() const;
    std::shared_ptr<Batch> ReadBatch();
    std::shared_ptr<Batch> ReadBatchColumns(const std::vector<std::string>& columns_names);

private:
    std::ifstream fin_;
    std::shared_ptr<Schema> schema_;
    size_t rows_number_;
    size_t columns_number_;
    size_t batches_number_;
    size_t schema_offset_;
    size_t current_batch_;
    bool schema_read_;
};

class WriterClmn {
public:
    explicit WriterClmn(const std::string& file_clmn);
    ~WriterClmn();
    void WriteBatch(std::shared_ptr<Batch> batch);
    void WriteSchema(std::shared_ptr<Schema> schema);
    void WriteFooter();

private:
    std::ofstream fout_;
    size_t rows_number_;
    size_t batches_number_;
    size_t columns_number_;
    size_t schema_start_;
    bool schema_written_;
};
