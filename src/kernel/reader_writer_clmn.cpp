#include "src/kernel/reader_writer_clmn.h"

std::shared_ptr<Column> ReadColumnFromClmn(std::ifstream& fin, Type type, size_t rows_number, size_t flags) {
    size_t encoded_size;
    fin.read(reinterpret_cast<char*>(&encoded_size), sizeof(size_t));
    std::vector<char> data(encoded_size);
    fin.read(reinterpret_cast<char*>(data.data()), encoded_size);
    return DecodeColumn(data, type, rows_number, flags);
}

void WriteColumnToClmn(std::ofstream& fout, std::shared_ptr<Column> column, Type type, size_t rows_number, size_t flags) {
    std::vector<char> encoded = EncodeColumn(column, type, rows_number, flags);
    size_t encoded_size = encoded.size();
    fout.write(reinterpret_cast<char*>(&encoded_size), sizeof(size_t));
    fout.write(reinterpret_cast<char*>(encoded.data()), encoded_size);
}

ReaderClmn::ReaderClmn(const std::string& file_clmn) {
    current_batch_ = 0;
    rows_number_ = 0;
    columns_number_ = 0;
    batches_number_ = 0;
    compression_flags_ = 0;
    schema_offset_ = 0;
    schema_read_ = false;
    fin_.open(file_clmn, std::ios::binary);
    if (!fin_.is_open()) {
        throw std::runtime_error("Cannot open file {" + file_clmn + "} :: ReaderClmn");
    }
    fin_.seekg(-5 * sizeof(size_t), std::ios::end);
    fin_.read(reinterpret_cast<char*>(&rows_number_), sizeof(size_t));
    fin_.read(reinterpret_cast<char*>(&columns_number_), sizeof(size_t));
    fin_.read(reinterpret_cast<char*>(&batches_number_), sizeof(size_t));
    fin_.read(reinterpret_cast<char*>(&compression_flags_), sizeof(size_t));
    fin_.read(reinterpret_cast<char*>(&schema_offset_), sizeof(size_t));
}

ReaderClmn::~ReaderClmn() {
    fin_.close();
}

void ReaderClmn::ReadSchema() {
    fin_.seekg(-schema_offset_, std::ios::end);
    std::vector<std::string> columns_names(columns_number_);
    std::vector<Type> columns_types(columns_number_);
    for (size_t i = 0; i < columns_number_; i++) {
        fin_.read(reinterpret_cast<char*>(&columns_types[i]), sizeof(uint8_t));
        size_t name_size;
        fin_.read(reinterpret_cast<char*>(&name_size), sizeof(size_t));
        columns_names[i].resize(name_size);
        fin_.read(&columns_names[i][0], name_size);
    }
    schema_ = std::make_shared<Schema>(std::move(columns_names), std::move(columns_types));
    schema_read_ = true;
    fin_.seekg(0, std::ios::beg);
}

std::shared_ptr<Schema> ReaderClmn::GetSchema() const {
    if (!schema_read_) {
        throw std::runtime_error("ReadSchema must be called before GetSchema :: ReaderClmn");
    }
    return schema_;
}

std::shared_ptr<Batch> ReaderClmn::ReadBatch() {
    if (!schema_read_) {
        throw std::runtime_error("ReadSchema must be called before ReadBatch :: ReaderClmn");
    }
    if (current_batch_ >= batches_number_) {
        return nullptr;
    }
    size_t batch_start = static_cast<size_t>(fin_.tellg());
    size_t metadata_offset;
    fin_.read(reinterpret_cast<char*>(&metadata_offset), sizeof(size_t));
    fin_.seekg(batch_start + metadata_offset, std::ios::beg);
    size_t batch_rows_number;
    fin_.read(reinterpret_cast<char*>(&batch_rows_number), sizeof(size_t));
    std::vector<size_t> column_offsets(columns_number_);
    for (size_t i = 0; i < columns_number_; i++) {
        fin_.read(reinterpret_cast<char*>(&column_offsets[i]), sizeof(size_t));
    }
    size_t next_batch_pos = static_cast<size_t>(fin_.tellg());
    std::vector<std::shared_ptr<Column>> columns;
    columns.reserve(columns_number_);
    for (size_t i = 0; i < columns_number_; i++) {
        fin_.seekg(batch_start + column_offsets[i], std::ios::beg);
        columns.push_back(ReadColumnFromClmn(fin_, schema_->GetType(i), batch_rows_number, compression_flags_));
    }
    fin_.seekg(next_batch_pos, std::ios::beg);
    current_batch_++;
    return std::make_shared<Batch>(batch_rows_number, schema_, std::move(columns));
}

std::shared_ptr<Batch> ReaderClmn::ReadBatchColumns(const std::vector<std::string>& columns_names) {
    if (!schema_read_) {
        throw std::runtime_error("ReadSchema must be called before ReadBatchColumns :: ReaderClmn");
    }
    if (current_batch_ >= batches_number_) {
        return nullptr;
    }
    size_t batch_start = static_cast<size_t>(fin_.tellg());
    size_t metadata_offset;
    fin_.read(reinterpret_cast<char*>(&metadata_offset), sizeof(size_t));
    fin_.seekg(batch_start + metadata_offset, std::ios::beg);
    size_t batch_rows;
    fin_.read(reinterpret_cast<char*>(&batch_rows), sizeof(size_t));
    std::vector<size_t> column_offsets(columns_number_);
    for (size_t i = 0; i < columns_number_; i++) {
        fin_.read(reinterpret_cast<char*>(&column_offsets[i]), sizeof(size_t));
    }
    size_t next_batch_pos = static_cast<size_t>(fin_.tellg());
    std::vector<std::string> names;
    std::vector<Type> types;
    std::vector<std::shared_ptr<Column>> columns;
    for (const std::string& column_name : columns_names) {
        size_t index = schema_->FindColumn(column_name);
        if (index < columns_number_) {
            names.push_back(column_name);
            types.push_back(schema_->GetType(index));
            fin_.seekg(batch_start + column_offsets[index], std::ios::beg);
            columns.push_back(ReadColumnFromClmn(fin_, schema_->GetType(index), batch_rows, compression_flags_));
        }
    }
    fin_.seekg(next_batch_pos, std::ios::beg);
    current_batch_++;
    return std::make_shared<Batch>(batch_rows, std::make_shared<Schema>(std::move(names), std::move(types)), std::move(columns));
}

WriterClmn::WriterClmn(const std::string& file_clmn) {
    rows_number_ = 0;
    columns_number_ = 0;
    batches_number_ = 0;
    schema_start_ = 0;
    compression_flags_ = GetCompressionFlags();
    schema_written_ = false;
    fout_.open(file_clmn, std::ios::binary);
    if (!fout_.is_open()) {
        throw std::runtime_error("Cannot open file {" + file_clmn + "} :: WriterClmn");
    }
}

WriterClmn::~WriterClmn() {
    fout_.close();
}

void WriterClmn::WriteBatch(std::shared_ptr<Batch> batch) {
    if (schema_written_) {
        throw std::runtime_error("WriteBatch must be called before WriteSchema :: WriterClmn");
    }
    size_t batch_rows_number  = batch->GetRowsNumber();
    rows_number_ += batch_rows_number;
    batches_number_++;
    columns_number_ = batch->GetColumnsNumber();
    size_t batch_start = static_cast<size_t>(fout_.tellp());
    size_t metadata_offset = 0;
    fout_.write(reinterpret_cast<char*>(&metadata_offset), sizeof(size_t));
    std::vector<size_t> columns_offsets(columns_number_);
    for (size_t i = 0; i < columns_number_; i++) {
        columns_offsets[i] = static_cast<size_t>(fout_.tellp()) - batch_start;
        WriteColumnToClmn(fout_, batch->GetColumn(i), batch->GetType(i), batch_rows_number, compression_flags_);
    }
    metadata_offset = static_cast<size_t>(fout_.tellp()) - batch_start;
    fout_.write(reinterpret_cast<char*>(&batch_rows_number), sizeof(size_t));
    for (size_t i = 0; i < columns_number_; i++) {
        fout_.write(reinterpret_cast<char*>(&columns_offsets[i]), sizeof(size_t));
    }
    size_t end_pos = static_cast<size_t>(fout_.tellp());
    fout_.seekp(batch_start, std::ios::beg);
    fout_.write(reinterpret_cast<char*>(&metadata_offset), sizeof(size_t));
    fout_.seekp(end_pos);
}

void WriterClmn::WriteSchema(std::shared_ptr<Schema> schema) {
    if (schema_written_) {
        throw std::runtime_error("WriteSchema must be called only once :: WriterClmn");
    }
    columns_number_ = schema->GetColumnsNumber();
    schema_start_ = static_cast<size_t>(fout_.tellp());
    for (size_t i = 0; i < columns_number_; i++) {
        Type column_type = schema->GetType(i);
        fout_.write(reinterpret_cast<char*>(&column_type), sizeof(uint8_t));
        const std::string& column_name = schema->GetName(i);
        size_t column_name_size = column_name.size();
        fout_.write(reinterpret_cast<char*>(&column_name_size), sizeof(size_t));
        fout_.write(column_name.c_str(), column_name_size);
    }
    schema_written_ = true;
}

void WriterClmn::WriteFooter() {
    if (!schema_written_) {
        throw std::runtime_error("WriteSchema must be called before WriteFooter :: WriterClmn");
    }
    size_t schema_offset = static_cast<size_t>(fout_.tellp()) - schema_start_ + 5 * sizeof(size_t);
    fout_.write(reinterpret_cast<char*>(&rows_number_), sizeof(size_t));
    fout_.write(reinterpret_cast<char*>(&columns_number_), sizeof(size_t));
    fout_.write(reinterpret_cast<char*>(&batches_number_), sizeof(size_t));
    fout_.write(reinterpret_cast<char*>(&compression_flags_), sizeof(size_t));
    fout_.write(reinterpret_cast<char*>(&schema_offset), sizeof(size_t));
    fout_.flush();
}
