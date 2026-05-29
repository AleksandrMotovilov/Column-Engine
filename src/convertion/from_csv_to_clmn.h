#pragma once

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "src/batch.h"

std::pair<std::string, char> ReadWordFromCsv(std::ifstream& fin);

std::pair<std::vector<std::string>, char> ReadRowFromCsv(std::ifstream& fin);

void ConvertFromCsvToClmn(const std::string& file_csv, const std::string& file_scheme, const std::string& file_clmn);