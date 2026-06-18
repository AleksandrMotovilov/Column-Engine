#pragma once

#include <cstdint>
#include <vector>

int BitsRequired(uint64_t max_value);

template<typename U>
std::vector<char> BitPackUnsigned(const std::vector<U>& input, int bits) {
    size_t input_size = input.size();
    size_t output_size = (input_size * static_cast<size_t>(bits) + 7) / 8;
    std::vector<char> output(output_size, 0);
    unsigned __int128 buf = 0;
    int buf_bits = 0;
    size_t byte_pos = 0;
    for (size_t i = 0; i < input_size; i++) {
        buf |= (static_cast<unsigned __int128>(input[i]) << buf_bits);
        buf_bits += bits;
        while (buf_bits >= 8) {
            output[byte_pos] = static_cast<char>(static_cast<uint8_t>(buf));
            buf >>= 8;
            buf_bits -= 8;
            byte_pos++;
        }
    }
    if (buf_bits > 0) {
        output[byte_pos] = static_cast<char>(static_cast<uint8_t>(buf));
    }
    return output;
}

template<typename U>
std::vector<U> BitUnpackUnsigned(const char* input, size_t size, int bits) {
    std::vector<U> output(size);
    unsigned __int128 mask = (static_cast<unsigned __int128>(1) << bits) - 1;
    unsigned __int128 buf = 0;
    int buf_bits = 0;
    size_t byte_pos = 0;
    for (size_t i = 0; i < size; i++) {
        while (buf_bits < bits) {
            buf |= (static_cast<unsigned __int128>(static_cast<uint8_t>(input[byte_pos])) << buf_bits);
            buf_bits += 8;
            byte_pos++;
        }
        output[i] = static_cast<U>(static_cast<uint64_t>(buf & mask));
        buf >>= bits;
        buf_bits -= bits;
    }
    return output;
}

template<typename T>
size_t EstimateIntegerBitpack(const std::vector<T>& input, bool used_delta) {
    using U = std::make_unsigned_t<T>;

    size_t size = input.size();
    if (size == 0) {
        return 1;
    }
    if (used_delta) {
        U max_diff = 0;
        for (size_t i = 1; i < size; i++) {
            U udiff = static_cast<U>(input[i]);
            if (udiff > max_diff) {
                max_diff = udiff;
            }
        }
        int bits = 1;
        if (size > 1) {
            bits = BitsRequired(static_cast<uint64_t>(max_diff));
        }
        return sizeof(T) + 1 + ((size - 1) * static_cast<size_t>(bits) + 7) / 8;
    }
    U max_value = 0;
    for (T value : input) {
        U uvalue = static_cast<U>(value);
        if (uvalue > max_value) {
            max_value = uvalue;
        }
    }
    int bits = BitsRequired(static_cast<uint64_t>(max_value));
    return 1 + (size * static_cast<size_t>(bits) + 7) / 8;
}
