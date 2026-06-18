#pragma once

#include <cstring>
#include <type_traits>
#include <vector>
#include "src/compression/encoding_bitpack.h"

template<typename T>
void DoDelta(std::vector<T>& input) {
    using U = std::make_unsigned_t<T>;
    size_t size = input.size();
    for (size_t i = size - 1; i > 0; i--) {
        input[i] = static_cast<T>(static_cast<U>(input[i]) - static_cast<U>(input[i - 1]));
    }
}

template<typename T>
void UndoDelta(std::vector<T>& input) {
    using U = std::make_unsigned_t<T>;
    size_t size = input.size();
    for (size_t i = 1; i < size; i++) {
        input[i] = static_cast<T>(static_cast<U>(input[i]) + static_cast<U>(input[i - 1]));
    }
}

template<typename T>
std::vector<char> EncodeDelta(const std::vector<T>& input, bool use_bitpack) {
    using U = std::make_unsigned_t<T>;
    size_t size = input.size();
    std::vector<T> delta = input;
    DoDelta<T>(delta);
    if (use_bitpack && size > 1) {
        T first = delta[0];
        U max_diff = 0;
        for (size_t i = 1; i < size; i++) {
            U udiff = static_cast<U>(delta[i]);
            if (udiff > max_diff) {
                max_diff = udiff;
            }
        }
        int bits = BitsRequired(static_cast<uint64_t>(max_diff));
        std::vector<U> udiffs(size - 1);
        for (size_t i = 1; i < size; i++) {
            udiffs[i - 1] = static_cast<U>(delta[i]);
        }
        std::vector<char> packed = BitPackUnsigned<U>(udiffs, bits);
        std::vector<char> output(sizeof(T) + 1 + packed.size());
        std::memcpy(output.data(), &first, sizeof(T));
        output[sizeof(T)] = static_cast<char>(bits);
        std::memcpy(output.data() + sizeof(T) + 1, packed.data(), packed.size());
        return output;
    }
    const char* raw = reinterpret_cast<const char*>(delta.data());
    return std::vector<char>(raw, raw + size * sizeof(T));
}

template<typename T>
std::vector<T> DecodeDelta(const std::vector<char>& input, size_t& offset, size_t size, bool use_bitpack) {
    using U = std::make_unsigned_t<T>;
    std::vector<T> output(size);
    if (use_bitpack && size > 1) {
        std::memcpy(&output[0], input.data() + offset, sizeof(T));
        offset += sizeof(T);
        int bits = static_cast<int>(static_cast<uint8_t>(*(input.data() + offset)));
        offset += 1;
        size_t packed_size = ((size - 1) * static_cast<size_t>(bits) + 7) / 8;
        std::vector<U> udiffs = BitUnpackUnsigned<U>(input.data() + offset, size - 1, bits);
        offset += packed_size;
        for (size_t i = 1; i < size; i++) {
            output[i] = static_cast<T>(udiffs[i - 1]);
        }
    } else {
        std::memcpy(output.data(), input.data() + offset, size * sizeof(T));
        offset += size * sizeof(T);
    }
    UndoDelta<T>(output);
    return output;
}
