#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <lz4.h>
#include "src/kernel/column.h"

constexpr size_t kFlagRLE = 1u;
constexpr size_t kFlagDelta = 2u;
constexpr size_t kFlagDict = 4u;
constexpr size_t kFlagLZ4 = 8u;
constexpr size_t kFlagBitPack = 16u;

size_t GetCompressionFlags();

std::vector<char> EncodeColumn(std::shared_ptr<Column> column, Type type, size_t rows_number, size_t flags);

std::shared_ptr<Column> DecodeColumn(std::vector<char>& data, Type type, size_t rows_number, size_t flags);
