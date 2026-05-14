#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <iostream>
#include "objects.h"

std::pair<std::string, char> ReadWordFromCsv(std::ifstream& fin);

std::pair<std::vector<std::string>, char> ReadRowFromCsv(std::ifstream& fin);

void ConvertFromCsvToClmn(const std::string& file_csv, const std::string& file_scheme, const std::string& file_clmn);