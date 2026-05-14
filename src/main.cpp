#include "from_csv_to_clmn.h"
#include "from_clmn_to_csv.h"

int main() {
    ConvertFromCsvToClmn("data.csv", "schema.csv", "data.clmn");
    ConvertFromClmnToCsv("data.clmn", "schema_new.csv", "data_new.csv");
}

// g++ main.cpp from_csv_to_clmn.cpp from_clmn_to_csv.cpp objects.cpp  -o main