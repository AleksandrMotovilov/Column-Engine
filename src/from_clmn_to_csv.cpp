#include "from_clmn_to_csv.h"
#include <iostream>
#include <sstream>

//----------------------CLMNReader----------------------
CLMNReader::CLMNReader(const std::string& file, std::shared_ptr<Schema> schema) {
    schema_ = schema;
    fin_ = std::ifstream(file);
    if (!fin_.is_open()) {
        throw std::runtime_error("Cannot open file {" + file + "}: CLMNReader::CLMNReader");
    }
}
std::shared_ptr<Table> CLMNReader::Read() {
    std::shared_ptr<Table> table = std::make_shared<Table>(schema_);
    int64_t offset;
    fin_.seekg(-sizeof(offset), std::ios::end);
    fin_.read(reinterpret_cast<char*>(&offset), sizeof(offset));
    fin_.seekg(0, std::ios::end);
    std::streampos fin_end = fin_.tellg();
    std::streampos schema_start = fin_end - static_cast<std::streampos>(offset);
    fin_.seekg(0, std::ios::beg);
    size_t ind = 0;
    std::string line;
    std::streampos pos = fin_.tellg();
    size_t expected_number_of_columns = schema_->ColumnsNumber();
    while (pos < schema_start && std::getline(fin_, line, '\n')) {
        pos = fin_.tellg();
        std::stringstream ss;
        ss << line;
        std::string s;
        while (std::getline(ss, s, ',')) {
            table->Push(ind, s, schema_->GetType(ind));
        }
        ind++;
    }
    if (ind != expected_number_of_columns) {
        throw std::runtime_error("Iinvalid number of columns {columns " + std::to_string(ind + 1) + ", expected " +  std::to_string(expected_number_of_columns) + "}: CSVReader::Read");
     }
    return table;
}
CLMNReader::~CLMNReader() {
    fin_.close();
}

//----------------------CSVWriter----------------------
CSVWriter::CSVWriter(const std::string& file, std::shared_ptr<Schema> schema) {
    schema_ = schema;
    fout_.open(file);
    if (!fout_ .is_open()) {
        throw std::runtime_error("Cannot open file {" + file + "}: CSVWriter::CSVWriter");
    }
}
void CSVWriter::Write(std::shared_ptr<Table> table) {
    size_t n = table->RowsNumber();
    size_t m = table->ColumnsNumber();
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < m; j++) {
            fout_ << table->GetValueString(i, j);
            if (j != m - 1) {
                fout_ << ',';
            }
        }
        fout_ << '\n';
    }
    fout_.flush();
}
void CSVWriter::WriteSchema(const std::string& file) {
    std::ofstream fout(file);
    fout << schema_->GetString();
    fout.close();
}
CSVWriter::~CSVWriter() {
    fout_.close();
}

//----------------------ConvertClmnToCsv----------------------
void ConvertFromClmnToCsv(const std::string& file_clmn, const std::string& file_scv, const std::string& file_scheme) {
    std::cout << "Converting " << file_clmn << " to " << file_scv << " with scheme " << file_scheme << std::endl;
    std::shared_ptr<Schema> schema = std::make_shared<Schema>();
    schema->FromCLMN(file_clmn);
    CLMNReader reader(file_clmn, schema);
    std::shared_ptr<Table> table(reader.Read());
    CSVWriter writer(file_scv, schema);
    writer.Write(table);
    writer.WriteSchema(file_scheme);
}