#pragma once

#include <cstring>
#include <stdexcept>
#include <vector>
#include <lz4.h>

std::vector<char> Lz4Compress(const std::vector<char>& input);

std::vector<char> Lz4Decompress(const std::vector<char>& input);
