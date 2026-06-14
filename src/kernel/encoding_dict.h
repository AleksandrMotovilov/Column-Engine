#pragma once

#include <cstring>
#include <unordered_map>
#include <utility>
#include <vector>
#include "src/kernel/encoding_base.h"
#include "src/kernel/encoding_bitpack.h"

template<typename T>
std::vector<char> EncodeDict(const std::vector<T>& input, bool use_bitpack) {
    std::vector<T> dict;
    std::unordered_map<T, uint32_t> dict_map;
    size_t size = input.size();
    std::vector<uint32_t> codes(size);
    for (size_t i = 0; i < size; i++) {
        auto it = dict_map.find(input[i]);
        if (it == dict_map.end()) {
            uint32_t index = static_cast<uint32_t>(dict.size());
            dict_map[input[i]] = index;
            dict.push_back(input[i]);
            codes[i] = index;
        } else {
            codes[i] = it->second;
        }
    }
    size_t dict_size = dict.size();
    std::vector<char> output;
    const char* ptr = reinterpret_cast<const char*>(&dict_size);
    output.insert(output.end(), ptr, ptr + sizeof(size_t));
    WriteValues<T>(output, dict);
    if (use_bitpack && dict_size > 0) {
        int bits = BitsRequired(static_cast<uint64_t>(dict_size - 1));
        std::vector<char> packed_codes = BitPackUnsigned<uint32_t>(codes, bits);
        output.push_back(static_cast<char>(bits));
        output.insert(output.end(), packed_codes.begin(), packed_codes.end());
    } else {
        ptr = reinterpret_cast<const char*>(codes.data());
        output.insert(output.end(), ptr, ptr + size * sizeof(uint32_t));
    }
    return output;
}

template<typename T>
std::vector<T> DecodeDict(
    const std::vector<char>& input, size_t& offset, size_t size, bool use_bitpack) {
    size_t dict_size;
    std::memcpy(&dict_size, input.data() + offset, sizeof(size_t));
    offset += sizeof(size_t);
    std::vector<T> dict = ReadValues<T>(input, offset, dict_size);
    std::vector<uint32_t> codes(size);
    if (use_bitpack && dict_size > 0) {
        int bits = static_cast<int>(static_cast<uint8_t>(input[offset]));
        offset += 1;
        size_t packed_size = (size * static_cast<size_t>(bits) + 7) / 8;
        codes = BitUnpackUnsigned<uint32_t>(input.data() + offset, size, bits);
        offset += packed_size;
    } else {
        std::memcpy(codes.data(), input.data() + offset, size * sizeof(uint32_t));
        offset += size * sizeof(uint32_t);
    }
    std::vector<T> output(size);
    for (size_t i = 0; i < size; i++) {
        output[i] = dict[codes[i]];
    }
    return output;
}

template<typename T>
std::pair<size_t, size_t> EstimateDict(const std::vector<T>& input) {
    size_t size = input.size();
    if (size == 0) {
        return {sizeof(size_t), sizeof(size_t)};
    }
    std::unordered_map<T, uint32_t> dict_map;
    size_t dict_values_bytes = sizeof(size_t);
    for (size_t i = 0; i < size; i++) {
        auto it = dict_map.find(input[i]);
        if (it == dict_map.end()) {
            uint32_t index = static_cast<uint32_t>(dict_map.size());
            dict_map[input[i]] = index;
            dict_values_bytes += ElementValueSize<T>(input[i]);
        }
    }
    size_t dict_size = dict_map.size();
    size_t without_bp = dict_values_bytes + size * sizeof(uint32_t);
    int bits = BitsRequired(static_cast<uint64_t>(dict_size - 1));
    size_t with_bp = dict_values_bytes + 1 + (size * static_cast<size_t>(bits) + 7) / 8;
    return {without_bp, with_bp};
}
