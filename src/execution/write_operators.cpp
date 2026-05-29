#include "src/execution/write_operators.h"

WriteOperator::WriteOperator(const std::string& file_clmn, std::shared_ptr<Operator> next) {
    next_ = std::move(next);
    fout_.open(file_clmn, std::ios::binary);
    if (!fout_.is_open()) {
        throw std::runtime_error("Cannot open file {" + file_clmn + "} :: WriteOperator");
    }
}

std::shared_ptr<Batch> WriteOperator::Next() {
    size_t rows_number = 0;
    size_t columns_number = 0;
    while (std::shared_ptr<Batch> batch = next_->Next()) {
        rows_number = batch->GetRowsNumber();
        size_t column_batch_size = batch->GetColumnsNumber();
        columns_number += column_batch_size;
        std::vector<Type> columns_types = batch->GetTypes();
        std::vector<std::string> columns_names = batch->GetNames();
        size_t batch_start = static_cast<size_t>(fout_.tellp());
        size_t metadata_offset = 0;
        fout_.write(reinterpret_cast<char*>(&metadata_offset), sizeof(size_t));
        std::vector<size_t> columns_offsets(column_batch_size);
        for (size_t i = 0; i < column_batch_size; i++) {
            columns_offsets[i] = static_cast<size_t>(fout_.tellp()) - batch_start;
            const Column& column = *batch->GetColumn(i);
            if (columns_types[i] == Type::Int16) {
                const std::vector<int16_t>& column_data = dynamic_cast<const ColumnTyped<int16_t>&>(column).GetData();
                fout_.write(reinterpret_cast<const char*>(column_data.data()), rows_number * sizeof(int16_t));
            } else if (columns_types[i] == Type::Int32) {
                const std::vector<int32_t>& column_data = dynamic_cast<const ColumnTyped<int32_t>&>(column).GetData();
                fout_.write(reinterpret_cast<const char*>(column_data.data()), rows_number * sizeof(int32_t));
            } else if (columns_types[i] == Type::Int64) {
                const std::vector<int64_t>& column_data = dynamic_cast<const ColumnTyped<int64_t>&>(column).GetData();
                fout_.write(reinterpret_cast<const char*>(column_data.data()), rows_number * sizeof(int64_t));
            } else if (columns_types[i] == Type::Int128) {
                // TODO ??????????????????????????????????????????????????????????????????????????????????????????
            } else if (columns_types[i] == Type::Float) {
                const std::vector<float>& column_data = dynamic_cast<const ColumnTyped<float>&>(column).GetData();
                fout_.write(reinterpret_cast<const char*>(column_data.data()), rows_number * sizeof(float));
            } else if (columns_types[i] == Type::Double) {
                const std::vector<double>& column_data = dynamic_cast<const ColumnTyped<double>&>(column).GetData();
                fout_.write(reinterpret_cast<const char*>(column_data.data()), rows_number * sizeof(double));
            } else if (columns_types[i] == Type::Date) {
                const std::vector<Date>& column_data = dynamic_cast<const ColumnTyped<Date>&>(column).GetData();
                std::vector<int32_t> raw(rows_number);
                for (size_t j = 0; j < rows_number; j++) {
                    raw[j] = column_data[j].GetValue();
                }
                fout_.write(reinterpret_cast<const char*>(raw.data()), rows_number * sizeof(int32_t));
            } else if (columns_types[i] == Type::Timestamp) {
                const std::vector<Timestamp>& column_data = dynamic_cast<const ColumnTyped<Timestamp>&>(column).GetData();
                std::vector<int64_t> raw(rows_number);
                for (size_t j = 0; j < rows_number; j++) {
                    raw[j] = column_data[j].GetValue();
                }
                fout_.write(reinterpret_cast<const char*>(raw.data()), rows_number * sizeof(int64_t));
            } else if (columns_types[i] == Type::Char) {
                const std::vector<char>& column_data = dynamic_cast<const ColumnTyped<char>&>(column).GetData();
                fout_.write(column_data.data(), rows_number * sizeof(char));
            } else if (columns_types[i] == Type::String) {
                const std::vector<std::string>& column_data = dynamic_cast<const ColumnTyped<std::string>&>(column).GetData();
                for (size_t j = 0; j < rows_number; j++) {
                    size_t str_size = column_data[j].size();
                    fout_.write(reinterpret_cast<char*>(&str_size), sizeof(size_t));
                    fout_.write(column_data[j].c_str(), str_size * sizeof(char));
                }
            } else {
                throw std::runtime_error("Invalid type :: WriteOperator");
            }
        }
        metadata_offset = static_cast<size_t>(fout_.tellp()) - batch_start;
        fout_.write(reinterpret_cast<char*>(&column_batch_size), sizeof(size_t));
        for (size_t i = 0; i < column_batch_size; i++) {
            fout_.write(reinterpret_cast<char*>(&columns_offsets[i]), sizeof(size_t));
            fout_.write(reinterpret_cast<char*>(&columns_types[i]), sizeof(uint8_t));
            size_t name_size = columns_names[i].size();
            fout_.write(reinterpret_cast<char*>(&name_size), sizeof(size_t));
            fout_.write(columns_names[i].c_str(), name_size * sizeof(char));
        }
        std::streampos end_pos = fout_.tellp();
        fout_.seekp(batch_start, std::ios::beg);
        fout_.write(reinterpret_cast<char*>(&metadata_offset), sizeof(size_t));
        fout_.seekp(end_pos);
    }
    fout_.write(reinterpret_cast<char*>(&rows_number), sizeof(size_t));
    fout_.write(reinterpret_cast<char*>(&columns_number), sizeof(size_t));
    fout_.close();
    return nullptr;
}
