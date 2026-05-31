#include "src/kernel/reader_writer_clmn.h"

static std::shared_ptr<Column> ReadColumnFromClmn(std::ifstream& fin, Type column_type, size_t rows_number) {
    switch (column_type) {
        case Type::Int16: {
            std::vector<int16_t> column_data(rows_number);
            fin.read(reinterpret_cast<char*>(column_data.data()), rows_number * sizeof(int16_t));
            return std::make_shared<ColumnTyped<int16_t>>(std::move(column_data));
        }
        case Type::Int32: {
            std::vector<int32_t> column_data(rows_number);
            fin.read(reinterpret_cast<char*>(column_data.data()), rows_number * sizeof(int32_t));
            return std::make_shared<ColumnTyped<int32_t>>(std::move(column_data));
        }
        case Type::Int64: {
            std::vector<int64_t> column_data(rows_number);
            fin.read(reinterpret_cast<char*>(column_data.data()), rows_number * sizeof(int64_t));
            return std::make_shared<ColumnTyped<int64_t>>(std::move(column_data));
        }
        case Type::Float: {
            std::vector<float> column_data(rows_number);
            fin.read(reinterpret_cast<char*>(column_data.data()), rows_number * sizeof(float));
            return std::make_shared<ColumnTyped<float>>(std::move(column_data));
        }
        case Type::Double: {
            std::vector<double> column_data(rows_number);
            fin.read(reinterpret_cast<char*>(column_data.data()), rows_number * sizeof(double));
            return std::make_shared<ColumnTyped<double>>(std::move(column_data));
        }
        case Type::Date: {
            std::vector<int32_t> raw(rows_number);
            fin.read(reinterpret_cast<char*>(raw.data()), rows_number * sizeof(int32_t));
            std::vector<Date> column_data(rows_number);
            for (size_t i = 0; i < rows_number; i++) {
                column_data[i] = Date{raw[i]};
            }
            return std::make_shared<ColumnTyped<Date>>(std::move(column_data));
        }
        case Type::Timestamp: {
            std::vector<int64_t> raw(rows_number);
            fin.read(reinterpret_cast<char*>(raw.data()), rows_number * sizeof(int64_t));
            std::vector<Timestamp> column_data(rows_number);
            for (size_t i = 0; i < rows_number; i++) {
                column_data[i] = Timestamp{raw[i]};
            }
            return std::make_shared<ColumnTyped<Timestamp>>(std::move(column_data));
        }
        case Type::Char: {
            std::vector<char> column_data(rows_number);
            fin.read(column_data.data(), rows_number * sizeof(char));
            return std::make_shared<ColumnTyped<char>>(std::move(column_data));
        }
        case Type::String: {
            std::vector<std::string> column_data(rows_number);
            for (size_t i = 0; i < rows_number; i++) {
                size_t str_size;
                fin.read(reinterpret_cast<char*>(&str_size), sizeof(size_t));
                column_data[i].resize(str_size);
                fin.read(&column_data[i][0], str_size);
            }
            return std::make_shared<ColumnTyped<std::string>>(std::move(column_data));
        }
        default:
            throw std::runtime_error("Unsupported type :: ReadColumnFromClmn");
    }
}

static void WriteColumnToClmn(std::ofstream& fout, std::shared_ptr<Column> column, Type column_type, size_t rows_number) {
    switch (column_type) {
        case Type::Int16: {
            const std::vector<int16_t>& column_data = dynamic_cast<const ColumnTyped<int16_t>&>(*column).GetData();
            fout.write(reinterpret_cast<const char*>(column_data.data()), rows_number * sizeof(int16_t));
            break;
        }
        case Type::Int32: {
            const std::vector<int32_t>& column_data = dynamic_cast<const ColumnTyped<int32_t>&>(*column).GetData();
            fout.write(reinterpret_cast<const char*>(column_data.data()), rows_number * sizeof(int32_t));
            break;
        }
        case Type::Int64: {
            const std::vector<int64_t>& column_data = dynamic_cast<const ColumnTyped<int64_t>&>(*column).GetData();
            fout.write(reinterpret_cast<const char*>(column_data.data()), rows_number * sizeof(int64_t));
            break;
        }
        case Type::Float: {
            const std::vector<float>& column_data = dynamic_cast<const ColumnTyped<float>&>(*column).GetData();
            fout.write(reinterpret_cast<const char*>(column_data.data()), rows_number * sizeof(float));
            break;
        }
        case Type::Double: {
            const std::vector<double>& column_data = dynamic_cast<const ColumnTyped<double>&>(*column).GetData();
            fout.write(reinterpret_cast<const char*>(column_data.data()), rows_number * sizeof(double));
            break;
        }
        case Type::Date: {
            const std::vector<Date>& column_data = dynamic_cast<const ColumnTyped<Date>&>(*column).GetData();
            std::vector<int32_t> raw(rows_number);
            for (size_t i = 0; i < rows_number; i++) {
                raw[i] = column_data[i].GetValue();
            }
            fout.write(reinterpret_cast<const char*>(raw.data()), rows_number * sizeof(int32_t));
            break;
        }
        case Type::Timestamp: {
            const std::vector<Timestamp>& column_data = dynamic_cast<const ColumnTyped<Timestamp>&>(*column).GetData();
            std::vector<int64_t> raw(rows_number);
            for (size_t i = 0; i < rows_number; i++) {
                raw[i] = column_data[i].GetValue();
            }
            fout.write(reinterpret_cast<const char*>(raw.data()), rows_number * sizeof(int64_t));
            break;
        }
        case Type::Char: {
            const std::vector<char>& column_data = dynamic_cast<const ColumnTyped<char>&>(*column).GetData();
            fout.write(column_data.data(), rows_number * sizeof(char));
            break;
        }
        case Type::String: {
            const std::vector<std::string>& column_data = dynamic_cast<const ColumnTyped<std::string>&>(*column).GetData();
            for (size_t i = 0; i < rows_number; i++) {
                size_t str_size = column_data[i].size();
                fout.write(reinterpret_cast<const char*>(&str_size), sizeof(size_t));
                fout.write(column_data[i].c_str(), str_size);
            }
            break;
        }
        default:
            throw std::runtime_error("Unsupported type :: WriteColumnToClmn");
    }
}

ReaderClmn::ReaderClmn(const std::string& file_clmn) {
    current_batch_ = 0;
    rows_number_ = 0;
    columns_number_ = 0;
    batches_number_ = 0;
    schema_offset_ = 0;
    schema_read_ = false;
    fin_.open(file_clmn, std::ios::binary);
    if (!fin_.is_open()) {
        throw std::runtime_error("Cannot open file {" + file_clmn + "} :: ReaderClmn");
    }
    fin_.seekg(-4 * static_cast<std::streamoff>(sizeof(size_t)), std::ios::end);
    fin_.read(reinterpret_cast<char*>(&rows_number_), sizeof(size_t));
    fin_.read(reinterpret_cast<char*>(&columns_number_), sizeof(size_t));
    fin_.read(reinterpret_cast<char*>(&batches_number_), sizeof(size_t));
    fin_.read(reinterpret_cast<char*>(&schema_offset_), sizeof(size_t));
}

ReaderClmn::~ReaderClmn() {
    fin_.close();
}

void ReaderClmn::ReadSchema() {
    fin_.seekg(-static_cast<std::streamoff>(schema_offset_), std::ios::end);
    std::vector<std::string> names(columns_number_);
    std::vector<Type> types(columns_number_);
    for (size_t i = 0; i < columns_number_; i++) {
        fin_.read(reinterpret_cast<char*>(&types[i]), sizeof(uint8_t));
        size_t name_size;
        fin_.read(reinterpret_cast<char*>(&name_size), sizeof(size_t));
        names[i].resize(name_size);
        fin_.read(&names[i][0], name_size);
    }
    schema_ = std::make_shared<Schema>(std::move(names), std::move(types));
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
    size_t batch_rows;
    fin_.read(reinterpret_cast<char*>(&batch_rows), sizeof(size_t));
    std::vector<size_t> column_offsets(columns_number_);
    for (size_t i = 0; i < columns_number_; i++) {
        fin_.read(reinterpret_cast<char*>(&column_offsets[i]), sizeof(size_t));
    }
    size_t next_batch_pos = static_cast<size_t>(fin_.tellg());
    std::vector<std::shared_ptr<Column>> columns;
    columns.reserve(columns_number_);
    for (size_t i = 0; i < columns_number_; i++) {
        fin_.seekg(batch_start + column_offsets[i], std::ios::beg);
        columns.push_back(ReadColumnFromClmn(fin_, schema_->GetType(i), batch_rows));
    }
    fin_.seekg(next_batch_pos, std::ios::beg);
    current_batch_++;
    return std::make_shared<Batch>(batch_rows, schema_, std::move(columns));
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
    for (const std::string& name : columns_names) {
        size_t index = schema_->FindColumn(name);
        if (index < columns_number_) {
            names.push_back(name);
            types.push_back(schema_->GetType(index));
            fin_.seekg(batch_start + column_offsets[index], std::ios::beg);
            columns.push_back(ReadColumnFromClmn(fin_, schema_->GetType(index), batch_rows));
        }
    }
    fin_.seekg(next_batch_pos, std::ios::beg);
    current_batch_++;
    return std::make_shared<Batch>(batch_rows, std::make_shared<Schema>(std::move(names), std::move(types)), std::move(columns));
}

WriterClmn::WriterClmn(const std::string& file_clmn) {
    rows_number_ = 0;
    batches_number_ = 0;
    columns_number_ = 0;
    schema_start_ = 0;
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
    size_t batch_rows = batch->GetRowsNumber();
    rows_number_ += batch_rows;
    batches_number_++;
    columns_number_ = batch->GetColumnsNumber();

    size_t batch_start = static_cast<size_t>(fout_.tellp());
    size_t metadata_offset = 0;
    fout_.write(reinterpret_cast<char*>(&metadata_offset), sizeof(size_t));

    std::vector<size_t> columns_offsets(columns_number_);
    for (size_t i = 0; i < columns_number_; i++) {
        columns_offsets[i] = static_cast<size_t>(fout_.tellp()) - batch_start;
        WriteColumnToClmn(fout_, batch->GetColumn(i), batch->GetType(i), batch_rows);
    }

    metadata_offset = static_cast<size_t>(fout_.tellp()) - batch_start;
    fout_.write(reinterpret_cast<char*>(&batch_rows), sizeof(size_t));
    for (size_t i = 0; i < columns_number_; i++) {
        fout_.write(reinterpret_cast<char*>(&columns_offsets[i]), sizeof(size_t));
    }
    std::streampos end_pos = fout_.tellp();
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
        const std::string& name = schema->GetName(i);
        size_t name_size = name.size();
        fout_.write(reinterpret_cast<char*>(&name_size), sizeof(size_t));
        fout_.write(name.c_str(), name_size);
    }
    schema_written_ = true;
}


void WriterClmn::WriteFooter() {
    if (!schema_written_) {
        throw std::runtime_error("WriteSchema must be called before WriteFooter :: WriterClmn");
    }
    size_t schema_offset = static_cast<size_t>(fout_.tellp()) - schema_start_ + 4 * sizeof(size_t);
    fout_.write(reinterpret_cast<char*>(&rows_number_), sizeof(size_t));
    fout_.write(reinterpret_cast<char*>(&columns_number_), sizeof(size_t));
    fout_.write(reinterpret_cast<char*>(&batches_number_), sizeof(size_t));
    fout_.write(reinterpret_cast<char*>(&schema_offset), sizeof(size_t));
    fout_.flush();
}
