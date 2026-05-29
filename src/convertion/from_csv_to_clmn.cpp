#include "src/convertion/from_csv_to_clmn.h"

std::pair<std::string, char> ReadWordFromCsv(std::ifstream& fin) {
    int mode = 0;
    std::string s;
    char d;
    while (true) {
        int ic = fin.get();
        if (mode == 1) {
            if (ic == EOF) {
                throw std::runtime_error("Invalid format :: ReadWordFromCsv :: ConvertFromCsvToClmn");
            }
            char c = static_cast<char>(ic);
            if (c == '"') {
                mode = 0;
            } else {
                s += c;
            }
        } else if (mode == 0) {
            if (ic == EOF) {
                d = static_cast<char>(EOF);
                break;
            }
            char c = static_cast<char>(ic);
            if (c == '"') {
                mode = 1;
            } else if (c == ',') {
                d = ',';
                break;
            } else if (c == '\n') {
                d = '\n';
                break;
            } else {
                s += c;
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
        int ic = fin.get();
        if (mode == 1) {
            if (ic == EOF) {
                throw std::runtime_error("Invalid format :: ReadRowFromCsv :: ConvertFromCsvToClmn");
            }
            char c = static_cast<char>(ic);
            if (c == '"') {
                mode = 0;
            } else {
                s += c;
            }
        } else if (mode == 0) {
            if (ic == EOF) {
                row.push_back(s);
                d = static_cast<char>(EOF);
                break;
            }
            char c = static_cast<char>(ic);
            if (c == '"') {
                mode = 1;
            } else if (c == ',') {
                row.push_back(s);
                s = "";
            } else if (c == '\n') {
                row.push_back(s);
                d = '\n';
                break;
            } else {
                s += c;
            }
        } else {
            throw std::runtime_error("Invalid mode :: ReadRowFromCsv :: ConvertFromCsvToClmn");
        }
    }
    return std::make_pair(row, d);
}

void ConvertFromCsvToClmn(const std::string& file_csv, const std::string& file_scheme, const std::string& file_clmn) {
    std::cout << "Converting " << file_csv << " with scheme " << file_scheme << " to " << file_clmn << std::endl;

    std::vector<std::string> columns_names(0);
    std::vector<Type> columns_types(0);

    std::ifstream fin_schema(file_scheme);
    if (!fin_schema.is_open()) {
        throw std::runtime_error("Cannot open file {" + file_scheme + "} :: ConvertFromCsvToClmn");
    }
    while (true) {
        std::pair<std::string, char> s;
        s = ReadWordFromCsv(fin_schema);
        if (s.second == EOF) {
            if (columns_names.size() == 0) {
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
        columns_names.push_back(name);
        columns_types.push_back(FromString<Type>(type));
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
        bool is_end = (s.second == static_cast<char>(EOF) && s.first.size() == 1 && s.first[0].empty());
        if (!is_end) {
            rows_number++;
        }
        if (s.second == static_cast<char>(EOF)) {
            break;
        }
    }
    fin_csv.close();

    size_t columns_number = columns_names.size();

    size_t column_batch_index = 0;

    std::ofstream fout_clmn(file_clmn, std::ios::binary);
    if (!fout_clmn.is_open()) {
        throw std::runtime_error("Cannot open file {" + file_clmn + "} :: ConvertFromCsvToClmn");
    }
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

        size_t batch_start = static_cast<size_t>(fout_clmn.tellp());
        size_t metadata_offset = 0;
        fout_clmn.write(reinterpret_cast<char*>(&metadata_offset), sizeof(size_t));

        std::vector<size_t> columns_offsets(column_batch_size);
        for (size_t i = 0; i < column_batch_size; i++) {
            columns_offsets[i] = static_cast<size_t>(fout_clmn.tellp()) - batch_start;
            if (columns_types[column_batch_index + i] == Type::Int16) {
                std::vector<int16_t> value(rows_number);
                for (size_t j = 0; j < rows_number; j++) {
                    value[j] = FromString<int16_t>(columns[i][j]);
                }
                fout_clmn.write(reinterpret_cast<char*>(&value[0]), rows_number * sizeof(int16_t));
            } else if (columns_types[column_batch_index + i] == Type::Int32) {
                std::vector<int32_t> value(rows_number);
                for (size_t j = 0; j < rows_number; j++) {
                    value[j] = FromString<int32_t>(columns[i][j]);
                }
                fout_clmn.write(reinterpret_cast<char*>(&value[0]), rows_number * sizeof(int32_t));
            } else if (columns_types[column_batch_index + i] == Type::Int64) {
                std::vector<int64_t> value(rows_number);
                for (size_t j = 0; j < rows_number; j++) {
                    value[j] = FromString<int64_t>(columns[i][j]);
                }
                fout_clmn.write(reinterpret_cast<char*>(&value[0]), rows_number * sizeof(int64_t));
            } else if (columns_types[column_batch_index + i] == Type::Int128) {
                // TODO ??????????????????????????????????????????????????????????????????????????????????????????
            } else if (columns_types[column_batch_index + i] == Type::Float) {
                std::vector<float> value(rows_number);
                for (size_t j = 0; j < rows_number; j++) {
                    value[j] = FromString<float>(columns[i][j]);
                }
                fout_clmn.write(reinterpret_cast<char*>(&value[0]), rows_number * sizeof(float));
            } else if (columns_types[column_batch_index + i] == Type::Double) {
                std::vector<double> value(rows_number);
                for (size_t j = 0; j < rows_number; j++) {
                    value[j] = FromString<double>(columns[i][j]);
                }
                fout_clmn.write(reinterpret_cast<char*>(&value[0]), rows_number * sizeof(double));
            } else if (columns_types[column_batch_index + i] == Type::Date) {
                std::vector<int32_t> value(rows_number);
                for (size_t j = 0; j < rows_number; j++) {
                    value[j] = FromString<Date>(columns[i][j]).GetValue();
                }
                fout_clmn.write(reinterpret_cast<char*>(&value[0]), rows_number * sizeof(int32_t));
            } else if (columns_types[column_batch_index + i] == Type::Timestamp) {
                std::vector<int64_t> value(rows_number);
                for (size_t j = 0; j < rows_number; j++) {
                    value[j] = FromString<Timestamp>(columns[i][j]).GetValue();
                }
                fout_clmn.write(reinterpret_cast<char*>(&value[0]), rows_number * sizeof(int64_t));
            } else if (columns_types[column_batch_index + i] == Type::Char) {
                std::vector<char> value(rows_number);
                for (size_t j = 0; j < rows_number; j++) {
                    value[j] = FromString<char>(columns[i][j]);
                }
                fout_clmn.write(reinterpret_cast<char*>(&value[0]), rows_number * sizeof(char));
            } else if (columns_types[column_batch_index + i] == Type::String) {
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

        metadata_offset = static_cast<size_t>(fout_clmn.tellp()) - batch_start;
        fout_clmn.write(reinterpret_cast<char*>(&column_batch_size), sizeof(size_t));
        for (size_t i = 0; i < column_batch_size; i++) {
            fout_clmn.write(reinterpret_cast<char*>(&columns_offsets[i]), sizeof(size_t));
            fout_clmn.write(reinterpret_cast<char*>(&columns_types[column_batch_index + i]), sizeof(uint8_t));
            size_t name_size = columns_names[column_batch_index + i].size();
            fout_clmn.write(reinterpret_cast<char*>(&name_size), sizeof(size_t));
            fout_clmn.write(columns_names[column_batch_index + i].c_str(), name_size);
        }

        std::streampos end_pos = fout_clmn.tellp();
        fout_clmn.seekp(batch_start, std::ios::beg);
        fout_clmn.write(reinterpret_cast<char*>(&metadata_offset), sizeof(size_t));
        fout_clmn.seekp(end_pos);

        column_batch_index = column_batch_index + column_batch_size;
    }

    fout_clmn.write(reinterpret_cast<char*>(&rows_number), sizeof(size_t));
    fout_clmn.write(reinterpret_cast<char*>(&columns_number), sizeof(size_t));
    fout_clmn.close();
}
