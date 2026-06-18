#pragma once

#include <iostream>
#include <string>
#include "src/kernel/batch.h"
#include "src/convertion/reader_writer_csv.h"
#include "src/convertion/reader_writer_clmn.h"

void ConvertFromCsvToClmn(const std::string& file_csv, const std::string& file_scheme, const std::string& file_clmn);
