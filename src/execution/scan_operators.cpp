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
            return std::make_shared<Batch>(rows_number_, std::vector<std::string>{}, std::vector<Type>{}, std::vector<std::shared_ptr<Column>>{});
        }
        return nullptr;
    }
    if (columns_returned_ >= available_columns_.size()) {
        return nullptr;
    }
    size_t start = columns_returned_;
    size_t end = std::min(start + kColumnBatchSize, available_columns_.size());
    columns_returned_ = end;
    std::vector<Type> batch_types;
    std::vector<std::string> batch_names;
    for (size_t i = start; i < end; i++) {
        batch_names.push_back(available_columns_[i]);
        batch_types.push_back(columns_types_.at(available_columns_[i]));
    }
    std::vector<std::shared_ptr<Column>> columns;
    for (size_t i = 0; i < batch_names.size(); i++) {
        fin_.seekg(columns_data_pos_.at(batch_names[i]), std::ios::beg);
        Type column_type = batch_types[i];
        if (column_type == Type::Int16) {
            std::vector<int16_t> column_data(rows_number_);
            fin_.read(reinterpret_cast<char*>(column_data.data()), rows_number_ * sizeof(int16_t));
            columns.push_back(std::make_shared<ColumnTyped<int16_t>>(std::move(column_data)));
        } else if (column_type == Type::Int32) {
            std::vector<int32_t> column_data(rows_number_);
            fin_.read(reinterpret_cast<char*>(column_data.data()), rows_number_ * sizeof(int32_t));
            columns.push_back(std::make_shared<ColumnTyped<int32_t>>(std::move(column_data)));
        } else if (column_type == Type::Int64) {
            std::vector<int64_t> column_data(rows_number_);
            fin_.read(reinterpret_cast<char*>(column_data.data()), rows_number_ * sizeof(int64_t));
            columns.push_back(std::make_shared<ColumnTyped<int64_t>>(std::move(column_data)));
        } else if (column_type == Type::Int128) {
            // TODO ??????????????????????????????????????????????????????????????????????????????????????????
        } else if (column_type == Type::Float) {
            std::vector<float> column_data(rows_number_);
            fin_.read(reinterpret_cast<char*>(column_data.data()), rows_number_ * sizeof(float));
            columns.push_back(std::make_shared<ColumnTyped<float>>(std::move(column_data)));
        } else if (column_type == Type::Double) {
            std::vector<double> column_data(rows_number_);
            fin_.read(reinterpret_cast<char*>(column_data.data()), rows_number_ * sizeof(double));
            columns.push_back(std::make_shared<ColumnTyped<double>>(std::move(column_data)));
        } else if (column_type == Type::Date) {
            std::vector<int32_t> raw(rows_number_);
            fin_.read(reinterpret_cast<char*>(raw.data()), rows_number_ * sizeof(int32_t));
            std::vector<Date> column_data(rows_number_);
            for (size_t j = 0; j < rows_number_; j++) {
                column_data[j] = Date{raw[j]};
            }
            columns.push_back(std::make_shared<ColumnTyped<Date>>(std::move(column_data)));
        } else if (column_type == Type::Timestamp) {
            std::vector<int64_t> raw(rows_number_);
            fin_.read(reinterpret_cast<char*>(raw.data()), rows_number_ * sizeof(int64_t));
            std::vector<Timestamp> column_data(rows_number_);
            for (size_t j = 0; j < rows_number_; j++) {
                column_data[j] = Timestamp{raw[j]};
            }
            columns.push_back(std::make_shared<ColumnTyped<Timestamp>>(std::move(column_data)));
        } else if (column_type == Type::Char) {
            std::vector<char> column_data(rows_number_);
            fin_.read(column_data.data(), rows_number_ * sizeof(char));
            columns.push_back(std::make_shared<ColumnTyped<char>>(std::move(column_data)));
        } else if (column_type == Type::String) {
            std::vector<std::string> column_data(rows_number_);
            size_t str_size;
            for (size_t j = 0; j < rows_number_; j++) {
                fin_.read(reinterpret_cast<char*>(&str_size), sizeof(size_t));
                column_data[j].resize(str_size);
                fin_.read(&column_data[j][0], str_size * sizeof(char));
            }
            columns.push_back(std::make_shared<ColumnTyped<std::string>>(std::move(column_data)));
        } else {
            throw std::runtime_error("Invalid type :: ScanOperator");
        }
    }
    return std::make_shared<Batch>(rows_number_, std::move(batch_names), std::move(batch_types), std::move(columns));
}
