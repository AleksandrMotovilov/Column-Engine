#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "src/kernel/batch.h"
#include "src/kernel/reader_writer_clmn.h"
#include "src/kernel/reader_writer_csv.h"

void ConvertFromClmnToCsv(const std::string& file_clmn, const std::string& file_scheme, const std::string& file_csv);
