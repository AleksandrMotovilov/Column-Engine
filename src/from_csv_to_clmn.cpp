#include "from_csv_to_clmn.h"
#include <iostream>
#include <sstream>

//----------------------CSVReader----------------------
CSVReader::CSVReader(const std::string& file, std::shared_ptr<Schema> schema) {
    schema_ = schema;
    fin_.open(file);
    if (!fin_.is_open()) {
        throw std::runtime_error("Cannot open file {" + file + "}: CSVReader::CSVReader");
    }
}
std::shared_ptr<Table> CSVReader::Read() {
    std::shared_ptr<Table> table = std::make_shared<Table>(schema_);
    std::string line;
    size_t expected_number_of_columns = schema_->ColumnsNumber();
    while (std::getline(fin_, line, '\n')) {
        std::stringstream ss;
        ss << line;
        std::string s;
        size_t ind = 0;
        while (std::getline(ss, s, ',')) {
            table->Push(ind, s, schema_->GetType(ind));
            ind++;
        }
        if (ind != expected_number_of_columns) {
            throw std::runtime_error("Iinvalid number of columns {columns " + std::to_string(ind) + ", expected " +  std::to_string(expected_number_of_columns) + "}: CSVReader::Read");
        }
    }
    return table;
}
CSVReader::~CSVReader() {
    fin_.close();
}

//----------------------CLMNWriter----------------------
CLMNWriter::CLMNWriter(const std::string& file, std::shared_ptr<Schema> schema) {
    schema_ = schema;
    fout_ = std::ofstream(file);
    if (!fout_ .is_open()) {
        throw std::runtime_error("Cannot open file {" + file + "}: CLMNWriter::CLMNWriter");
    }
}
void CLMNWriter::Write(std::shared_ptr<Table> table) {
    size_t n = table->RowsNumber();
    size_t m = table->ColumnsNumber();
    for (size_t j = 0; j < m; j++) {
        for (size_t i = 0; i < n; i++) {
            fout_ << table->GetValueString(i, j);
            if (i != n - 1) {
                fout_ << ',';
            }
        }
        fout_ << '\n';
    }
    fout_.flush();
}
void CLMNWriter::WriteSchema() {
    int64_t offset = 8;
    std::string s = schema_->GetString();
    fout_ << s;
    offset = offset + s.size();
    fout_.write(reinterpret_cast<char*>(&offset), sizeof(offset));
    fout_.flush();
}
CLMNWriter::~CLMNWriter() {
    fout_.close();
}

//----------------------onvertCsvToClmn----------------------
void ConvertFromCsvToClmn(const std::string& file_csv, const std::string& file_scheme, const std::string& file_clmn) {
    std::cout << "Converting " << file_csv << " with scheme " << file_scheme << " to " << file_clmn << std::endl;
    std::shared_ptr<Schema> schema = std::make_shared<Schema>();
    schema->FromCSV(file_scheme);
    CSVReader reader(file_csv, schema);
    std::shared_ptr<Table> table(reader.Read());
    CLMNWriter writer(file_clmn, schema);
    writer.Write(table);
    writer.WriteSchema();
}