#include "src/kernel/reader_writer_csv.h"
#include "src/kernel/column_utils.h"

static std::pair<std::string, int> ReadWordFromCsv(std::ifstream& fin) {
    int mode = 0;
    std::string s;
    int d;
    while (true) {
        int ic = fin.get();
        if (mode == 1) {
            if (ic == EOF) {
                throw std::runtime_error("Invalid format :: ReadWordFromCsv");
            }
            char c = static_cast<char>(ic);
            if (c == '"') {
                mode = 0;
            } else {
                s += c;
            }
        } else {
            if (ic == EOF) {
                d = EOF;
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
        }
    }
    return {s, d};
}

static std::pair<std::vector<std::string>, int> ReadRowFromCsv(std::ifstream& fin) {
    std::vector<std::string> row;
    int mode = 0;
    std::string s;
    int d;
    while (true) {
        int ic = fin.get();
        if (mode == 1) {
            if (ic == EOF) {
                throw std::runtime_error("Invalid format :: ReadRowFromCsv");
            }
            char c = static_cast<char>(ic);
            if (c == '"') {
                mode = 0;
            } else {
                s += c;
            }
        } else {
            if (ic == EOF) {
                row.push_back(s);
                d = EOF;
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
        }
    }
    return {row, d};
}

ReaderCsv::ReaderCsv(const std::string& file_csv, const std::string& file_scheme) {
    file_scheme_ = file_scheme;
    done_ = false;
    fin_.open(file_csv);
    if (!fin_.is_open()) {
        throw std::runtime_error("Cannot open file {" + file_csv + "} :: ReaderCsv");
    }
}

ReaderCsv::~ReaderCsv() {
    fin_.close();
}

void ReaderCsv::ReadSchema() {
    std::ifstream fin_scheme(file_scheme_);
    if (!fin_scheme.is_open()) {
        throw std::runtime_error("Cannot open file {" + file_scheme_ + "} :: ReaderCsv");
    }
    std::vector<std::string> names;
    std::vector<Type> types;
    while (true) {
        std::pair<std::string, int> name_result = ReadWordFromCsv(fin_scheme);
        std::string name = name_result.first;
        int d1 = name_result.second;
        if (d1 == EOF) {
            if (names.empty()) {
                break;
            }
            throw std::runtime_error("Invalid scheme format :: ReaderCsv");
        }
        if (d1 != ',') {
            throw std::runtime_error("Invalid scheme format :: ReaderCsv");
        }
        std::pair<std::string, int> type_result = ReadWordFromCsv(fin_scheme);
        std::string type = type_result.first;
        int d2 = type_result.second;
        names.push_back(name);
        types.push_back(FromString<Type>(type));
        if (d2 == EOF) {
            break;
        }
        if (d2 != '\n') {
            throw std::runtime_error("Invalid scheme format :: ReaderCsv");
        }
    }
    schema_ = std::make_shared<Schema>(std::move(names), std::move(types));
}

std::shared_ptr<Schema> ReaderCsv::GetSchema() const {
    return schema_;
}

std::vector<std::vector<std::string>> ReaderCsv::ReadRowsStrings() {
    size_t columns_number = schema_->GetColumnsNumber();
    std::vector<std::vector<std::string>> columns_strings(columns_number);
    size_t row_index = 0;

    while (row_index < kRowBatchSize) {
        std::pair<std::vector<std::string>, int> row_result = ReadRowFromCsv(fin_);
        std::vector<std::string> row = row_result.first;
        int delim = row_result.second;
        bool is_phantom = (delim == EOF && row.size() == 1 && row[0].empty());
        if (is_phantom) {
            done_ = true;
            break;
        }
        for (size_t i = 0; i < columns_number; i++) {
            columns_strings[i].push_back(row[i]);
        }
        row_index++;
        if (delim == EOF) {
            done_ = true;
            break;
        }
    }
    return columns_strings;
}

std::shared_ptr<Batch> ReaderCsv::ReadBatch() {
    if (done_) {
        return nullptr;
    }
    std::vector<std::vector<std::string>> columns_strings = ReadRowsStrings();
    size_t rows_number = 0;
    if (!columns_strings.empty()) {
        rows_number = columns_strings[0].size();
    }
    if (rows_number == 0) {
        return nullptr;
    }
    size_t columns_number = schema_->GetColumnsNumber();
    std::vector<std::shared_ptr<Column>> columns;
    for (size_t i = 0; i < columns_number; i++) {
        columns.push_back(MakeColumnFromStrings(schema_->GetType(i), columns_strings[i]));
    }
    return std::make_shared<Batch>(rows_number, schema_, std::move(columns));
}

WriterCsv::WriterCsv(const std::string& file_csv, const std::string& file_scheme) {
    file_scheme_ = file_scheme;
    fout_csv_.open(file_csv);
    if (!fout_csv_.is_open()) {
        throw std::runtime_error("Cannot open file {" + file_csv + "} :: WriterCsv");
    }
}

WriterCsv::~WriterCsv() {
    fout_csv_.close();
}

void WriterCsv::WriteBatch(std::shared_ptr<Batch> batch) {
    size_t rows = batch->GetRowsNumber();
    size_t cols = batch->GetColumnsNumber();

    std::vector<std::vector<std::string>> col_strings(cols, std::vector<std::string>(rows));
    for (size_t i = 0; i < cols; i++) {
        Type type = batch->GetType(i);
        for (size_t j = 0; j < rows; j++) {
            col_strings[i][j] = GetStringValueAt(batch->GetColumn(i), type, j);
        }
    }

    for (size_t row = 0; row < rows; row++) {
        for (size_t col = 0; col < cols; col++) {
            const std::string& val = col_strings[col][row];
            if (val.find(',') != std::string::npos) {
                fout_csv_ << '"' << val << '"';
            } else {
                fout_csv_ << val;
            }
            if (col + 1 < cols) {
                fout_csv_ << ",";
            }
        }
        fout_csv_ << "\n";
    }
}

void WriterCsv::WriteSchema(std::shared_ptr<Schema> schema) {
    std::ofstream fout_scheme(file_scheme_);
    if (!fout_scheme.is_open()) {
        throw std::runtime_error("Cannot open file {" + file_scheme_ + "} :: WriterCsv");
    }
    size_t columns_number = schema->GetColumnsNumber();
    for (size_t i = 0; i < columns_number; i++) {
        fout_scheme << schema->GetName(i) << "," << ToString<Type>(schema->GetType(i));
        if (i + 1 != columns_number) {
            fout_scheme << "\n";
        }
    }
}

void WriterCsv::WriteFooter() {
    fout_csv_.flush();
}
