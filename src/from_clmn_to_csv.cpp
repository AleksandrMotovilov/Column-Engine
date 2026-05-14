#include "from_clmn_to_csv.h"

void ConvertFromClmnToCsv(const std::string& file_clmn, const std::string& file_scheme, const std::string& file_csv) {
    std::cout << "Converting " << file_clmn << " to " << file_csv << " with scheme " << file_scheme << std::endl;

    std::ifstream fin_clmn(file_clmn, std::ios::binary);
    if (!fin_clmn.is_open()) {
        throw std::runtime_error("Cannot open file {" + file_clmn + "} :: ConvertFromClmnToCsv");
    }
    size_t rows_number;
    size_t columns_number;
    fin_clmn.read(reinterpret_cast<char*>(&rows_number), sizeof(size_t));
    fin_clmn.read(reinterpret_cast<char*>(&columns_number), sizeof(size_t));
    fin_clmn.close();

    std::vector<std::string> names(columns_number);
    std::vector<Type> types(columns_number);

    std::ofstream fout_csv(file_csv);
    if (!fout_csv.is_open()) {
        throw std::runtime_error("Cannot open file {" + file_csv + "} :: ConvertFromClmnToCsv");
    }

    size_t row_batch_index = 0;

    while (row_batch_index < rows_number) {
        size_t row_batch_size = std::min(kRowBatchSize, rows_number - row_batch_index);

        std::vector<std::vector<std::string>> rows(row_batch_size, std::vector<std::string>(columns_number));

        std::ifstream fin_clmn(file_clmn, std::ios::binary);
        if (!fin_clmn.is_open()) {
            throw std::runtime_error("Cannot open file {" + file_clmn + "} :: ConvertFromClmnToCsv");
        }
        fin_clmn.seekg(sizeof(size_t) + sizeof(size_t), std::ios::beg);
        size_t column_batch_index = 0;
        
        while (column_batch_index < columns_number) {
            size_t column_batch_size;
            size_t column_batch_index_check;

            fin_clmn.read(reinterpret_cast<char*>(&column_batch_index_check), sizeof(size_t));
            fin_clmn.read(reinterpret_cast<char*>(&column_batch_size), sizeof(size_t));

            if (column_batch_index != column_batch_index_check) {
                throw std::runtime_error("Incorrect batch_index :: ConvertFromClmnToCsv");
            }

            std::vector<std::vector<std::string>> columns(column_batch_size, std::vector<std::string>(rows_number));
            for (size_t i = 0; i < column_batch_size; i++) {
                fin_clmn.read(reinterpret_cast<char*>(&types[column_batch_index + i]), sizeof(uint8_t));
                size_t size_str;
                fin_clmn.read(reinterpret_cast<char*>(&size_str), sizeof(size_t));
                std::string str(size_str, '\0');
                fin_clmn.read(&str[0], size_str * sizeof(char));
                names[column_batch_index + i] = str;
            }

            for (size_t i = 0; i < column_batch_size; i++) {
                if (types[column_batch_index + i] == Type::Int16) {
                    std::vector<int16_t> column(rows_number);
                    fin_clmn.read(reinterpret_cast<char*>(&column[0]), rows_number * sizeof(int16_t));
                    for (size_t j = 0; j < rows_number; j++) {
                        columns[i][j] = ToString<int16_t>(column[j]);
                    }
                } else if (types[column_batch_index + i] == Type::Int32) {
                    std::vector<int32_t> column(rows_number);
                    fin_clmn.read(reinterpret_cast<char*>(&column[0]), rows_number * sizeof(int32_t));
                    for (size_t j = 0; j < rows_number; j++) {
                        columns[i][j] = ToString<int32_t>(column[j]);
                    }
                } else if (types[column_batch_index + i] == Type::Int64) {
                    std::vector<int64_t> column(rows_number);
                    fin_clmn.read(reinterpret_cast<char*>(&column[0]), rows_number * sizeof(int64_t));
                    for (size_t j = 0; j < rows_number; j++) {
                        columns[i][j] = ToString<int64_t>(column[j]);
                    }
                } else if (types[column_batch_index + i] == Type::Int128) {
                    // TODO ??????????????????????????????????????????????????????????????????????????????????????????
                } else if (types[column_batch_index + i] == Type::Float) {
                    std::vector<float> column(rows_number);
                    fin_clmn.read(reinterpret_cast<char*>(&column[0]), rows_number * sizeof(float));
                    for (size_t j = 0; j < rows_number; j++) {
                        columns[i][j] = ToString<float>(column[j]);
                    }
                } else if (types[column_batch_index + i] == Type::Double) {
                    std::vector<double> column(rows_number);
                    fin_clmn.read(reinterpret_cast<char*>(&column[0]), rows_number * sizeof(double));
                    for (size_t j = 0; j < rows_number; j++) {
                        columns[i][j] = ToString<double>(column[j]);
                    }
                } else if (types[column_batch_index + i] == Type::Date) {
                    // TODO ??????????????????????????????????????????????????????????????????????????????????????????
                } else if (types[column_batch_index + i] == Type::Timestamp) {
                    // TODO ??????????????????????????????????????????????????????????????????????????????????????????
                } else if (types[column_batch_index + i] == Type::Char) {
                    std::vector<char> column(rows_number);
                    fin_clmn.read(reinterpret_cast<char*>(&column[0]), rows_number * sizeof(char));
                    for (size_t j = 0; j < rows_number; j++) {
                        columns[i][j] = ToString<char>(column[j]);
                    }
                } else if (types[column_batch_index + i] == Type::String) {
                    size_t size;
                    for (size_t j = 0; j < rows_number; j++) {
                        fin_clmn.read(reinterpret_cast<char*>(&size), sizeof(size_t));
                        std::string value(size, '\0');
                        fin_clmn.read(&value[0], size * sizeof(char));
                        columns[i][j] = ToString<std::string>(value);
                    }
                } else {
                    throw std::runtime_error("Invalid type :: ConvertFromClmnToCsv");
                }
            }

            for (size_t i = 0; i < row_batch_size; i++) {
                for (size_t j = 0; j < column_batch_size; j++) {
                    rows[i][column_batch_index + j] = columns[j][row_batch_index + i];
                }
            }

            column_batch_index = column_batch_index + column_batch_size;
        }
        fin_clmn.close();

        for (size_t i = 0; i < row_batch_size; i++) {
            for (size_t j = 0; j < columns_number; j++) {
                fout_csv << rows[i][j];
                if (j + 1 < columns_number) {
                    fout_csv << ",";
                } else {
                    if (row_batch_index + i + 1 < rows_number) {
                        fout_csv << "\n";
                    }
                }
            }
        }

        row_batch_index = row_batch_index + row_batch_size;
    }
    fout_csv.close();

    std::ofstream fout_scheme(file_scheme);
    if (!fout_scheme.is_open()) {
        throw std::runtime_error("Cannot open file {" + file_scheme + "} :: ConvertFromClmnToCsv");
    }
    for (size_t i = 0; i < columns_number; i++) {
        fout_scheme << names[i] << "," << StringFromType(types[i]);
        if (i + 1 != columns_number) {
            fout_scheme << "\n";
        }
    }
    fout_scheme.close();
}