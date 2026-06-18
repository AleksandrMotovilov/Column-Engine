#pragma once

#include <cstring>
#include <utility>
#include <vector>
#include "src/compression/encoding_base.h"
#include "src/compression/encoding_bitpack.h"

template<typename T>
std::vector<char> EncodeRle(const std::vector<T>& input, bool use_bitpack) {
    size_t size = input.size();
    std::vector<T> run_values;
    std::vector<size_t> run_counts;
    size_t i = 0;
    while (i < size) {
        size_t j = i + 1;
        while (j < size && input[j] == input[i]) {
            j++;
        }
        run_values.push_back(input[i]);
        run_counts.push_back(j - i);
        i = j;
    }
    size_t run_count = run_values.size();
    std::vector<char> output;
    const char* ptr = reinterpret_cast<const char*>(&run_count);
    output.insert(output.end(), ptr, ptr + sizeof(size_t));
    WriteValues<T>(output, run_values);
    if (use_bitpack && run_count > 0) {
        size_t max_count = 0;
        for (size_t count : run_counts) {
            if (count > max_count) {
                max_count = count;
            }
        }
        int bits = BitsRequired(static_cast<uint64_t>(max_count));
        std::vector<char> packed = BitPackUnsigned<size_t>(run_counts, bits);
        output.push_back(static_cast<char>(bits));
        output.insert(output.end(), packed.begin(), packed.end());
    } else {
        ptr = reinterpret_cast<const char*>(run_counts.data());
        output.insert(output.end(), ptr, ptr + run_count * sizeof(size_t));
    }
    return output;
}

template<typename T>
std::vector<T> DecodeRle(const std::vector<char>& input, size_t& offset, size_t size, bool use_bitpack) {
    size_t run_count;
    std::memcpy(&run_count, input.data() + offset, sizeof(size_t));
    offset += sizeof(size_t);
    std::vector<T> run_values = ReadValues<T>(input, offset, run_count);
    std::vector<size_t> run_counts(run_count);
    if (use_bitpack && run_count > 0) {
        int bits = static_cast<int>(static_cast<uint8_t>(input[offset]));
        offset += 1;
        size_t packed_size = (run_count * static_cast<size_t>(bits) + 7) / 8;
        run_counts = BitUnpackUnsigned<size_t>(input.data() + offset, run_count, bits);
        offset += packed_size;
    } else {
        std::memcpy(run_counts.data(), input.data() + offset, run_count * sizeof(size_t));
        offset += run_count * sizeof(size_t);
    }
    std::vector<T> output;
    output.reserve(size);
    for (size_t i = 0; i < run_count; i++) {
        for (size_t j = 0; j < run_counts[i]; j++) {
            output.push_back(run_values[i]);
        }
    }
    return output;
}

template<typename T>
std::pair<size_t, size_t> EstimateRle(const std::vector<T>& input) {
    size_t size = input.size();
    if (size == 0) {
        return {sizeof(size_t), sizeof(size_t)};
    }
    size_t run_count = 0;
    size_t max_count = 0;
    size_t run_values_bytes = 0;
    size_t i = 0;
    while (i < size) {
        size_t j = i + 1;
        while (j < size && input[j] == input[i]) {
            j++;
        }
        run_count++;
        run_values_bytes += ElementValueSize<T>(input[i]);
        size_t count = j - i;
        if (count > max_count) {
            max_count = count;
        }
        i = j;
    }
    int bits = BitsRequired(static_cast<uint64_t>(max_count));
    size_t without_bp = sizeof(size_t) + run_values_bytes + run_count * sizeof(size_t);
    size_t with_bp = sizeof(size_t) + run_values_bytes + 1 + (run_count * static_cast<size_t>(bits) + 7) / 8;
    return {without_bp, with_bp};
}
