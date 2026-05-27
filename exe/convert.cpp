#include <iostream>
#include <string>
#include "src/convertion/from_csv_to_clmn.h"

int main(int argc, char** argv) {
    std::string input_csv;
    std::string schema;
    std::string output_clmn;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--input" && i + 1 < argc) {
            input_csv = argv[++i];
        } else if (arg == "--schema" && i + 1 < argc) {
            schema = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            output_clmn = argv[++i];
        }
    }

    if (input_csv.empty() || schema.empty() || output_clmn.empty()) {
        std::cerr << "Usage: csv_to_columnar --input <csv> --schema <schema> --output <clmn>\n";
        return 1;
    }

    SetBatchSize(1000, 1000);
    ConvertFromCsvToClmn(input_csv, schema, output_clmn);
    return 0;
}
