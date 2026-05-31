#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "src/kernel/batch.h"

void ConvertFromClmnToCsv(const std::string& file_clmn, const std::string& file_scheme, const std::string& file_csv);
