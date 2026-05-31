#pragma once

#include <iostream>
#include <string>
#include "src/kernel/batch.h"

void ConvertFromCsvToClmn(const std::string& file_csv, const std::string& file_scheme, const std::string& file_clmn);
