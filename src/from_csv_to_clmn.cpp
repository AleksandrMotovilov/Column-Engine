#include "from_csv_to_clmn.h"
#include <cstddef>
#include <iostream>

std::pair<std::string, char> ReadWordFromCsv(std::ifstream& fin) {
    int mode = 0;
    std::string s;
    char d;
    while (true) {
        char c = fin.get();
        if (mode == 1) {
            if (c == '"') {
                mode = 0;
            } else if (c == EOF) {
                throw std::runtime_error("Invalid format :: ReadWordFromCsv :: ConvertFromCsvToClmn");
                break;
            }  else {
                s = s + c;
            }
        } else if (mode == 0) {
            if (c == '"') {
                mode = 1;
            } else if (c == EOF) {
                d = EOF;
                break;
            } else if (c == ',') {
                d = ',';
                break;
            } else if (c == '\n') {
                d = '\n';
                break;
            } else {
                s = s + c;
            }
        } else {
            throw std::runtime_error("Invalid mode :: ReadWordFromCsv :: ConvertFromCsvToClmn");
        }
    }
    return std::make_pair(s, d);
}

std::pair<std::vector<std::string>, char> ReadRowFromCsv(std::ifstream& fin) {
    std::vector<std::string> row = {};
    int mode = 0;
    std::string s;
    char d;
    while (true) {
        char c = fin.get();
        if (mode == 1) {
            if (c == '"') {
                mode = 0;
            } else if (c == EOF) {
                throw std::runtime_error("Invalid format :: ReadRowFromCsv :: ConvertFromCsvToClmn");
                break;
            }  else {
                s = s + c;
            }
        } else if (mode == 0) {
            if (c == '"') {
                mode = 1;
            } else if (c == EOF) {
                row.push_back(s);
                d = EOF;
                break;
            } else if (c == ',') {
                row.push_back(s);
                s = "";
            } else if (c == '\n') {
                row.push_back(s);
                d = '\n';
                break;
            } else {
                s = s + c;
            }
        } else {
            throw std::runtime_error("Invalid mode :: ReadRowFromCsv :: ConvertFromCsvToClmn");
        }
    }
    return std::make_pair(row, d);
}

void ConvertFromCsvToClmn(const std::string& file_csv, const std::string& file_scheme, const std::string& file_clmn) {
    std::cout << "Converting " << file_csv << " with scheme " << file_scheme << " to " << file_clmn << std::endl;

    std::vector<std::string> names(0);
    std::vector<Type> types(0);

    std::ifstream fin_schema(file_scheme);
    if (!fin_schema.is_open()) {
        throw std::runtime_error("Cannot open file {" + file_scheme + "} :: ConvertFromCsvToClmn");
    }
    while (true) {
        std::pair<std::string, char> s;
        s = ReadWordFromCsv(fin_schema);
        if (s.second == EOF) {
            if (names.size() == 0) {
                break;
            } else {
                throw std::runtime_error("Invalid format :: ConvertFromCsvToClmn");
            }
        }
        std::string name = s.first;
        if (s.second != ',') {
            throw std::runtime_error("Invalid format :: ConvertFromCsvToClmn");
        }
        s = ReadWordFromCsv(fin_schema);
        std::string type = s.first;
        if (s.second != '\n' && s.second != EOF) {
            throw std::runtime_error("Invalid format :: ConvertFromCsvToClmn");
        }
        names.push_back(name);
        types.push_back(TypeFromString(type));
        if (s.second == EOF) {
            break;
        }
    }
    fin_schema.close();

    std::ifstream fin_csv(file_csv);
    size_t rows_number = 0;
    while (true) {
        std::pair<std::vector<std::string>, char> s;
        s = ReadRowFromCsv(fin_csv);
        rows_number++;
        if (s.second == EOF) {
            break;
        }
    }
    fin_csv.close();

    size_t columns_number = names.size();

    size_t column_batch_index = 0;

    std::ofstream fout_clmn(file_clmn, std::ios::binary);
    if (!fout_clmn.is_open()) {
        throw std::runtime_error("Cannot open file {" + file_clmn + "} :: ConvertFromCsvToClmn");
    }

    fout_clmn.write(reinterpret_cast<char*>(&rows_number), sizeof(size_t));
    fout_clmn.write(reinterpret_cast<char*>(&columns_number), sizeof(size_t));

    while (column_batch_index < columns_number) {
        size_t column_batch_size = std::min(kColumnBatchSize, columns_number - column_batch_index);

        std::vector<std::vector<std::string>> columns(column_batch_size, std::vector<std::string>(rows_number));

        std::ifstream fin_csv(file_csv);
        if (!fin_csv.is_open()) {
            throw std::runtime_error("Cannot open file {" + file_csv + "} :: ConvertFromCsvToClmn");
        }
        size_t row_batch_index = 0;
        while (row_batch_index < rows_number) {
            size_t row_batch_size = 1;
            std::pair<std::vector<std::string>, char> s;
            s = ReadRowFromCsv(fin_csv);
            std::vector<std::string> row = s.first;
            for (size_t i = 0; i < column_batch_size; i++) {
                columns[i][row_batch_index] = row[column_batch_index + i];
            }
            row_batch_index = row_batch_index + row_batch_size;
        }
        fin_csv.close();

        fout_clmn.write(reinterpret_cast<char*>(&column_batch_index), sizeof(size_t));
        fout_clmn.write(reinterpret_cast<char*>(&column_batch_size), sizeof(size_t));

        for (size_t i = 0; i < column_batch_size; i++) {
            fout_clmn.write(reinterpret_cast<char*>(&types[column_batch_index + i]), sizeof(uint8_t));
            size_t size_str = names[column_batch_index + i].size();
            fout_clmn.write(reinterpret_cast<char*>(&size_str), sizeof(size_t));
            fout_clmn.write(names[column_batch_index + i].c_str(), size_str * sizeof(char));
        }

        for (size_t i = 0; i < column_batch_size; i++) {
            if (types[column_batch_index + i] == Type::Int16) {
                std::vector<int16_t> value(rows_number);
                for (size_t j = 0; j < rows_number; j++) {
                    value[j] = FromString<int16_t>(columns[i][j]);
                }
                fout_clmn.write(reinterpret_cast<char*>(&value[0]), rows_number * sizeof(int16_t));
            } else if (types[column_batch_index + i] == Type::Int32) {
                std::vector<int32_t> value(rows_number);
                for (size_t j = 0; j < rows_number; j++) {
                    value[j] = FromString<int32_t>(columns[i][j]);
                }
                fout_clmn.write(reinterpret_cast<char*>(&value[0]), rows_number * sizeof(int32_t));
            } else if (types[column_batch_index + i] == Type::Int64) {
                std::vector<int64_t> value(rows_number);
                for (size_t j = 0; j < rows_number; j++) {
                    value[j] = FromString<int64_t>(columns[i][j]);
                }
                fout_clmn.write(reinterpret_cast<char*>(&value[0]), rows_number * sizeof(int64_t));
            } else if (types[column_batch_index + i] == Type::Int128) {
                // TODO ??????????????????????????????????????????????????????????????????????????????????????????
            } else if (types[column_batch_index + i] == Type::Float) {
                std::vector<float> value(rows_number);
                for (size_t j = 0; j < rows_number; j++) {
                    value[j] = FromString<float>(columns[i][j]);
                }
                fout_clmn.write(reinterpret_cast<char*>(&value[0]), rows_number * sizeof(float));
            } else if (types[column_batch_index + i] == Type::Double) {
                std::vector<double> value(rows_number);
                for (size_t j = 0; j < rows_number; j++) {
                    value[j] = FromString<double>(columns[i][j]);
                }
                fout_clmn.write(reinterpret_cast<char*>(&value[0]), rows_number * sizeof(double));
            } else if (types[column_batch_index + i] == Type::Date) {
                // TODO ??????????????????????????????????????????????????????????????????????????????????????????
            } else if (types[column_batch_index + i] == Type::Timestamp) {
                // TODO ??????????????????????????????????????????????????????????????????????????????????????????
            } else if (types[column_batch_index + i] == Type::Char) {
                std::vector<char> value(rows_number);
                for (size_t j = 0; j < rows_number; j++) {
                    value[j] = FromString<char>(columns[i][j]);
                }
                fout_clmn.write(reinterpret_cast<char*>(&value[0]), rows_number * sizeof(char));
            } else if (types[column_batch_index + i] == Type::String) {
                size_t size;
                for (size_t j = 0; j < rows_number; j++) {
                    std::string value = FromString<std::string>(columns[i][j]);
                    size = value.size();
                    fout_clmn.write(reinterpret_cast<char*>(&size), sizeof(size_t));
                    fout_clmn.write(value.c_str(), size * sizeof(char));
                }
            } else {
                throw std::runtime_error("Invalid type :: ConvertFromCsvToClmn");
            }
        }

        std::cerr << "column " << column_batch_index << " " << rows_number << " " << columns_number << std::endl;
        for (size_t i = 0; i < column_batch_size; i++) {
            for (size_t j = 0; j < rows_number; j++) {
                std::cerr << columns[i][j] << ' ';
            }
            std::cerr << std::endl;
        }

        column_batch_index = column_batch_index + column_batch_size;
    }
    fout_clmn.close();
}