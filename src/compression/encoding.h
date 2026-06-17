#pragma once

#include <cstring>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include "src/kernel/column.h"
#include "src/kernel/types.h"
#include "src/compression/encoding_base.h"
#include "src/compression/encoding_bitpack.h"
#include "src/compression/encoding_lz4.h"
#include "src/compression/encoding_delta.h"
#include "src/compression/encoding_rle.h"
#include "src/compression/encoding_dict.h"

constexpr size_t kFlagRleInt = 1u;
constexpr size_t kFlagDeltaInt = 2u;
constexpr size_t kFlagDictInt = 4u;
constexpr size_t kFlagBitPackInt = 8u;
constexpr size_t kFlagRleStr = 16u;
constexpr size_t kFlagDictStr = 32u;
constexpr size_t kFlagBitPackStr = 64u;
constexpr size_t kFlagRleFloat = 128u;
constexpr size_t kFlagDictFloat = 256u;
constexpr size_t kFlagBitPackFloat = 512u;
constexpr size_t kFlagLz4 = 1024u;
constexpr size_t kFlagAdaptive = 2048u;

size_t GetCompressionFlags();

void choose_best_flags(size_t& best_flags, size_t& best_size, size_t flags, size_t size);

template<typename T>
std::vector<char> EncodeIntegerVector(const std::vector<T>& input, size_t flags) {
    using U = std::make_unsigned_t<T>;

    bool use_adaptive = (flags & kFlagAdaptive) != 0;

    if (use_adaptive) {
        size_t size = input.size();

        std::vector<T> delta_input = input;
        DoDelta<T>(delta_input);

        std::pair<size_t, size_t> rle = EstimateRle<T>(input);
        size_t rle_without_bp = rle.first;
        size_t rle_with_bp = rle.second;
    
        std::pair<size_t, size_t> rle_delta = EstimateRle<T>(delta_input);
        size_t rle_delta_without_bp = rle_delta.first;
        size_t rle_delta_with_bp = rle_delta.second;
    
        std::pair<size_t, size_t> dict = EstimateDict<T>(input);
        size_t dict_without_bp = dict.first;
        size_t dict_with_bp = dict.second;
    
        std::pair<size_t, size_t> dict_delta = EstimateDict<T>(delta_input);
        size_t dict_delta_without_bp = dict_delta.first;
        size_t dict_delta_with_bp = dict_delta.second;

        size_t best_size = EstimateRaw<T>(input);
        size_t best_flags = 0;
        choose_best_flags(best_flags, best_size, kFlagBitPackInt, EstimateIntegerBitpack<T>(input, false));
        choose_best_flags(best_flags, best_size, kFlagDeltaInt | kFlagBitPackInt, EstimateIntegerBitpack<T>(delta_input, true));
        choose_best_flags(best_flags, best_size, kFlagRleInt, rle_without_bp);
        choose_best_flags(best_flags, best_size, kFlagRleInt | kFlagBitPackInt, rle_with_bp);
        choose_best_flags(best_flags, best_size, kFlagDeltaInt | kFlagRleInt, rle_delta_without_bp);
        choose_best_flags(best_flags, best_size, kFlagDeltaInt | kFlagRleInt | kFlagBitPackInt, rle_delta_with_bp);
        choose_best_flags(best_flags, best_size, kFlagDictInt, dict_without_bp);
        choose_best_flags(best_flags, best_size, kFlagDictInt | kFlagBitPackInt, dict_with_bp);
        choose_best_flags(best_flags, best_size, kFlagDeltaInt | kFlagDictInt, dict_delta_without_bp);
        choose_best_flags(best_flags, best_size, kFlagDeltaInt | kFlagDictInt | kFlagBitPackInt, dict_delta_with_bp);

        flags = best_flags;
    }

    bool use_delta = (flags & kFlagDeltaInt) != 0;
    bool use_rle = (flags & kFlagRleInt) != 0;
    bool use_dict = (flags & kFlagDictInt) != 0;
    bool use_bitpack = (flags & kFlagBitPackInt) != 0;

    size_t size = input.size();
    std::vector<char> encoded;
    if (use_dict) {
        if (use_delta) {
            std::vector<T> delta_input = input;
            DoDelta<T>(delta_input);
            encoded = EncodeDict<T>(delta_input, use_bitpack);
        } else {
            encoded = EncodeDict<T>(input, use_bitpack);
        }
    } else if (use_rle) {
        if (use_delta) {
            std::vector<T> delta_input = input;
            DoDelta<T>(delta_input);
            encoded = EncodeRle<T>(delta_input, use_bitpack);
        } else {
            encoded = EncodeRle<T>(input, use_bitpack);
        }
    } else if (use_delta) {
        encoded = EncodeDelta<T>(input, use_bitpack);
    } else if (use_bitpack) {
        if (size > 0) {
            U max_value = 0;
            for (T value : input) {
                U uvalue = static_cast<U>(value);
                if (uvalue > max_value) {
                    max_value = uvalue;
                }
            }
            int bits = BitsRequired(static_cast<uint64_t>(max_value));
            std::vector<U> uvalues(size);
            for (size_t k = 0; k < size; k++) {
                uvalues[k] = static_cast<U>(input[k]);
            }
            std::vector<char> packed = BitPackUnsigned<U>(uvalues, bits);
            encoded.resize(1 + packed.size());
            encoded[0] = static_cast<char>(bits);
            std::memcpy(encoded.data() + 1, packed.data(), packed.size());
        }
    } else {
        const char* raw = reinterpret_cast<const char*>(input.data());
        encoded = std::vector<char>(raw, raw + size * sizeof(T));
    }

    if (use_adaptive) {
        std::vector<char> output(sizeof(size_t) + encoded.size());
        std::memcpy(output.data(), &flags, sizeof(size_t));
        std::memcpy(output.data() + sizeof(size_t), encoded.data(), encoded.size());
        return output;
    }
    return encoded;
}

template<typename T>
std::vector<T> DecodeIntegerVector(const std::vector<char>& input, size_t size, size_t flags) {
    using U = std::make_unsigned_t<T>;

    bool use_adaptive = (flags & kFlagAdaptive) != 0;

    size_t offset = 0;
    if (use_adaptive) {
        std::memcpy(&flags, input.data(), sizeof(size_t));
        offset = sizeof(size_t);
    }

    bool use_delta = (flags & kFlagDeltaInt) != 0;
    bool use_rle = (flags & kFlagRleInt) != 0;
    bool use_dict = (flags & kFlagDictInt) != 0;
    bool use_bitpack = (flags & kFlagBitPackInt) != 0;

    std::vector<T> output;
    if (use_dict) {
        output = DecodeDict<T>(input, offset, size, use_bitpack);
    } else if (use_rle) {
        output = DecodeRle<T>(input, offset, size, use_bitpack);
    } else if (use_delta) {
        return DecodeDelta<T>(input, offset, size, use_bitpack);
    } else if (use_bitpack && size > 0) {
        int bits = static_cast<int>(static_cast<uint8_t>(input[offset]));
        offset += 1;
        std::vector<U> uvalues = BitUnpackUnsigned<U>(input.data() + offset, size, bits);
        output.resize(size);
        for (size_t i = 0; i < size; i++) {
            output[i] = static_cast<T>(uvalues[i]);
        }
        return output;
    } else {
        output.resize(size);
        std::memcpy(output.data(), input.data() + offset, size * sizeof(T));
        return output;
    }

    if (use_delta) {
        UndoDelta<T>(output);
    }
    return output;
}

template<typename T>
std::vector<char> EncodeFloatVector(const std::vector<T>& input, size_t flags) {
    bool use_adaptive = (flags & kFlagAdaptive) != 0;

    if (use_adaptive) {
        size_t size = input.size();

        std::pair<size_t, size_t> rle = EstimateRle<T>(input);
        size_t rle_without_bp = rle.first;
        size_t rle_with_bp = rle.second;
    
        std::pair<size_t, size_t> dict = EstimateDict<T>(input);
        size_t dict_without_bp = dict.first;
        size_t dict_with_bp = dict.second;

        size_t best_size = EstimateRaw<T>(input);
        size_t best_flags = 0;
        choose_best_flags(best_flags, best_size, kFlagRleFloat, rle_without_bp);
        choose_best_flags(best_flags, best_size, kFlagRleFloat | kFlagBitPackFloat, rle_with_bp);
        choose_best_flags(best_flags, best_size, kFlagDictFloat, dict_without_bp);
        choose_best_flags(best_flags, best_size, kFlagDictFloat | kFlagBitPackFloat, dict_with_bp);

        flags = best_flags;
    }

    bool use_rle = (flags & kFlagRleFloat) != 0;
    bool use_dict = (flags & kFlagDictFloat) != 0;
    bool use_bitpack = (flags & kFlagBitPackFloat) != 0;

    std::vector<char> encoded;
    if (use_dict) {
        encoded = EncodeDict<T>(input, use_bitpack);
    } else if (use_rle) {
        encoded = EncodeRle<T>(input, use_bitpack);
    } else {
        const char* raw = reinterpret_cast<const char*>(input.data());
        encoded = std::vector<char>(raw, raw + input.size() * sizeof(T));
    }

    if (use_adaptive) {
        std::vector<char> output(sizeof(size_t) + encoded.size());
        std::memcpy(output.data(), &flags, sizeof(size_t));
        std::memcpy(output.data() + sizeof(size_t), encoded.data(), encoded.size());
        return output;
    }
    return encoded;
}

template<typename T>
std::vector<T> DecodeFloatVector(const std::vector<char>& input, size_t size, size_t flags) {
    bool use_adaptive = (flags & kFlagAdaptive) != 0;

    size_t offset = 0;
    if (use_adaptive) {
        std::memcpy(&flags, input.data(), sizeof(size_t));
        offset += sizeof(size_t);
    }

    bool use_rle = (flags & kFlagRleFloat) != 0;
    bool use_dict = (flags & kFlagDictFloat) != 0;
    bool use_bitpack = (flags & kFlagBitPackFloat) != 0;

    if (use_dict) {
        return DecodeDict<T>(input, offset, size, use_bitpack);
    }
    if (use_rle) {
        return DecodeRle<T>(input, offset, size, use_bitpack);
    }
    std::vector<T> output(size);
    std::memcpy(output.data(), input.data() + offset, size * sizeof(T));
    return output;
}

std::vector<char> EncodeStringVector(const std::vector<std::string>& input, size_t flags);

std::vector<std::string> DecodeStringVector(const std::vector<char>& input, size_t size, size_t flags);

std::vector<char> EncodeColumn(std::shared_ptr<Column> column, Type type, size_t rows_number, size_t flags);

std::shared_ptr<Column> DecodeColumn(std::vector<char>& input, Type type, size_t rows_number, size_t flags);
