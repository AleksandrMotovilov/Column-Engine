#pragma once

#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <cstdint>
#include "src/kernel/batch.h"
#include "src/compression/encoding.h"

std::shared_ptr<Column> ReadColumnFromClmn(std::ifstream& fin, Type type, size_t rows_number, size_t flags);

void WriteColumnToClmn(std::ofstream& fout, std::shared_ptr<Column> column, Type type, size_t rows_number, size_t flags);

class ReaderClmn {
public:
    explicit ReaderClmn(const std::string& file_clmn, std::optional<std::vector<std::string>> columns_names);
    ~ReaderClmn();
    void ReadSchema();
    std::shared_ptr<Schema> GetSchema() const;
    std::shared_ptr<Batch> ReadBatch();

private:
    std::ifstream fin_;
    std::shared_ptr<Schema> schema_;
    std::optional<std::vector<std::string>> requested_columns_;
    std::vector<size_t> selected_indices_;
    size_t rows_number_;
    size_t columns_number_;
    size_t batches_number_;
    size_t compression_flags_;
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
    size_t columns_number_;
    size_t batches_number_;
    size_t schema_start_;
    size_t compression_flags_;
    bool schema_written_;
};
