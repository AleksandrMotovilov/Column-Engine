#include "src/execution/scan_operators.h"

ScanOperator::ScanOperator(const std::string& file_clmn, std::vector<std::string> needed_columns) {
    needed_columns_ = std::move(needed_columns);
    rows_number_ = 0;
    columns_number_ = 0;
    columns_returned_ = 0;
    fin_.open(file_clmn, std::ios::binary);
    if (!fin_.is_open()) {
        throw std::runtime_error("Cannot open file {" + file_clmn + "} :: ScanOperator");
    }
    fin_.seekg(-2 * sizeof(size_t), std::ios::end);
    fin_.read(reinterpret_cast<char*>(&rows_number_), sizeof(size_t));
    fin_.read(reinterpret_cast<char*>(&columns_number_), sizeof(size_t));
    if (needed_columns_.empty()) {
        return;
    }
    fin_.seekg(0, std::ios::beg);
    size_t column_batch_index = 0;
    while (column_batch_index < columns_number_) {
        size_t batch_start = static_cast<size_t>(fin_.tellg());
        size_t metadata_offset;
        fin_.read(reinterpret_cast<char*>(&metadata_offset), sizeof(size_t));
        fin_.seekg(batch_start + metadata_offset, std::ios::beg);
        size_t batch_size;
        fin_.read(reinterpret_cast<char*>(&batch_size), sizeof(size_t));
        for (size_t i = 0; i < batch_size; i++) {
            size_t column_offset;
            fin_.read(reinterpret_cast<char*>(&column_offset), sizeof(size_t));
            Type type;
            fin_.read(reinterpret_cast<char*>(&type), sizeof(uint8_t));
            size_t name_size;
            fin_.read(reinterpret_cast<char*>(&name_size), sizeof(size_t));
            std::string name(name_size, '\0');
            fin_.read(&name[0], name_size * sizeof(char));
            columns_data_pos_[name] = batch_start + column_offset;
            columns_types_[name] = type;
        }
        column_batch_index += batch_size;
    }
    for (const std::string& column : needed_columns_) {
        if (columns_data_pos_.find(column) != columns_data_pos_.end()) {
            available_columns_.push_back(column);
        }
    }
}

std::shared_ptr<Batch> ScanOperator::Next() {
    if (available_columns_.empty()) {
        if (columns_returned_ == 0) {
            columns_returned_ = 1;
            return std::make_shared<Batch>(rows_number_, 0, std::vector<Type>{}, std::vector<std::string>{});
        }
        return nullptr;
    }
    if (columns_returned_ >= available_columns_.size()) {
        return nullptr;
    }
    size_t start = columns_returned_;
    size_t end = std::min(start + kColumnBatchSize, available_columns_.size());
    columns_returned_ = end;
    std::vector<Type> columns_types;
    std::vector<std::string> columns_names;
    for (size_t i = start; i < end; i++) {
        columns_names.push_back(available_columns_[i]);
        columns_types.push_back(columns_types_.at(available_columns_[i]));
    }
    std::shared_ptr<Batch> batch = std::make_shared<Batch>(rows_number_, columns_names.size(), columns_types, columns_names);
    for (size_t i = 0; i < columns_names.size(); i++) {
        fin_.seekg(columns_data_pos_.at(columns_names[i]), std::ios::beg);
        Type column_type = columns_types_.at(columns_names[i]);
        if (column_type == Type::Int16) {
            std::vector<int16_t> column(rows_number_);
            fin_.read(reinterpret_cast<char*>(&column[0]), rows_number_ * sizeof(int16_t));
            for (size_t j = 0; j < rows_number_; j++) {
                batch->SetValue(j, i, ToString<int16_t>(column[j]));
            }
        } else if (column_type == Type::Int32) {
            std::vector<int32_t> column(rows_number_);
            fin_.read(reinterpret_cast<char*>(&column[0]), rows_number_ * sizeof(int32_t));
            for (size_t j = 0; j < rows_number_; j++) {
                batch->SetValue(j, i, ToString<int32_t>(column[j]));
            }
        } else if (column_type == Type::Int64) {
            std::vector<int64_t> column(rows_number_);
            fin_.read(reinterpret_cast<char*>(&column[0]), rows_number_ * sizeof(int64_t));
            for (size_t j = 0; j < rows_number_; j++) {
                batch->SetValue(j, i, ToString<int64_t>(column[j]));
            }
        } else if (column_type == Type::Int128) {
            // TODO ??????????????????????????????????????????????????????????????????????????????????????????
        } else if (column_type == Type::Float) {
            std::vector<float> column(rows_number_);
            fin_.read(reinterpret_cast<char*>(&column[0]), rows_number_ * sizeof(float));
            for (size_t j = 0; j < rows_number_; j++) {
                batch->SetValue(j, i, ToString<float>(column[j]));
            }
        } else if (column_type == Type::Double) {
            std::vector<double> column(rows_number_);
            fin_.read(reinterpret_cast<char*>(&column[0]), rows_number_ * sizeof(double));
            for (size_t j = 0; j < rows_number_; j++) {
                batch->SetValue(j, i, ToString<double>(column[j]));
            }
        } else if (column_type == Type::Date) {
            std::vector<int32_t> column(rows_number_);
            fin_.read(reinterpret_cast<char*>(&column[0]), rows_number_ * sizeof(int32_t));
            for (size_t j = 0; j < rows_number_; j++) {
                batch->SetValue(j, i, ToString<Date>(Date(column[j])));
            }
        } else if (column_type == Type::Timestamp) {
            std::vector<int64_t> column(rows_number_);
            fin_.read(reinterpret_cast<char*>(&column[0]), rows_number_ * sizeof(int64_t));
            for (size_t j = 0; j < rows_number_; j++) {
                batch->SetValue(j, i, ToString<Timestamp>(Timestamp(column[j])));
            }
        } else if (column_type == Type::Char) {
            std::vector<char> column(rows_number_);
            fin_.read(reinterpret_cast<char*>(&column[0]), rows_number_ * sizeof(char));
            for (size_t j = 0; j < rows_number_; j++) {
                batch->SetValue(j, i, ToString<char>(column[j]));
            }
        } else if (column_type == Type::String) {
            size_t str_size;
            for (size_t j = 0; j < rows_number_; j++) {
                fin_.read(reinterpret_cast<char*>(&str_size), sizeof(size_t));
                std::string value(str_size, '\0');
                fin_.read(&value[0], str_size * sizeof(char));
                batch->SetValue(j, i, value);
            }
        } else {
            throw std::runtime_error("Invalid type :: ScanOperator");
        }
    }
    return batch;
}
