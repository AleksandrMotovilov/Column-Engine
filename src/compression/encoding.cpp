#include "src/compression/encoding.h"

size_t GetCompressionFlags() {
    size_t flags = 0;
#ifdef ENABLE_LZ4
    flags |= kFlagLz4;
#endif
#ifdef ENABLE_ADAPTIVE
    flags |= kFlagAdaptive;
    return flags;
#endif
#ifdef ENABLE_RLE_INT
    flags |= kFlagRleInt;
#endif
#ifdef ENABLE_DELTA_INT
    flags |= kFlagDeltaInt;
#endif
#ifdef ENABLE_DICT_INT
    flags |= kFlagDictInt;
#endif
#ifdef ENABLE_DICT_FLOAT
    flags |= kFlagDictFloat;
#endif
#ifdef ENABLE_RLE_FLOAT
    flags |= kFlagRleFloat;
#endif
#ifdef ENABLE_DICT_STR
    flags |= kFlagDictStr;
#endif
#ifdef ENABLE_RLE_STR
    flags |= kFlagRleStr;
#endif
#ifdef ENABLE_BITPACK_INT
    flags |= kFlagBitPackInt;
#endif
#ifdef ENABLE_BITPACK_FLOAT
    if (flags & (kFlagRleFloat | kFlagDictFloat)) {
        flags |= kFlagBitPackFloat;
    }
#endif
#ifdef ENABLE_BITPACK_STR
    if (flags & (kFlagRleStr | kFlagDictStr)) {
        flags |= kFlagBitPackStr;
    }
#endif
    return flags;
}

void ChooseBestFlags(size_t& best_flags, size_t& best_size, size_t flags, size_t size) {
    if (size < best_size) {
        best_size = size;
        best_flags = flags;
    }
}

std::vector<char> EncodeStringVector(const std::vector<std::string>& input, size_t flags) {
    bool use_adaptive = (flags & kFlagAdaptive) != 0;

    if (use_adaptive) {
        std::pair<size_t, size_t> rle = EstimateRle<std::string>(input);
        size_t rle_without_bp = rle.first;
        size_t rle_with_bp = rle.second;

        std::pair<size_t, size_t> dict = EstimateDict<std::string>(input);
        size_t dict_without_bp = dict.first;
        size_t dict_with_bp = dict.second;

        size_t best_size = EstimateRaw<std::string>(input);
        size_t best_flags = 0;
        ChooseBestFlags(best_flags, best_size, kFlagRleStr, rle_without_bp);
        ChooseBestFlags(best_flags, best_size, kFlagRleStr | kFlagBitPackStr, rle_with_bp);
        ChooseBestFlags(best_flags, best_size, kFlagDictStr, dict_without_bp);
        ChooseBestFlags(best_flags, best_size, kFlagDictStr | kFlagBitPackStr, dict_with_bp);

        flags = best_flags;
    }

    bool use_rle = (flags & kFlagRleStr) != 0;
    bool use_dict = (flags & kFlagDictStr) != 0;
    bool use_bitpack = (flags & kFlagBitPackStr) != 0;

    std::vector<char> encoded;
    if (use_rle) {
        encoded = EncodeRle<std::string>(input, use_bitpack);
    } else if (use_dict) {
        encoded = EncodeDict<std::string>(input, use_bitpack);
    } else {
        for (const std::string& str : input) {
            size_t str_size = str.size();
            const char* ptr = reinterpret_cast<const char*>(&str_size);
            encoded.insert(encoded.end(), ptr, ptr + sizeof(size_t));
            encoded.insert(encoded.end(), str.begin(), str.end());
        }
    }

    if (use_adaptive) {
        std::vector<char> output(sizeof(size_t) + encoded.size());
        std::memcpy(output.data(), &flags, sizeof(size_t));
        std::memcpy(output.data() + sizeof(size_t), encoded.data(), encoded.size());
        return output;
    }
    return encoded;
}

std::vector<std::string> DecodeStringVector(const std::vector<char>& input, size_t size, size_t flags) {
    bool use_adaptive = (flags & kFlagAdaptive) != 0;

    size_t offset = 0;

    if (use_adaptive) {
        std::memcpy(&flags, input.data(), sizeof(size_t));
        offset += sizeof(size_t);
    }

    bool use_rle = (flags & kFlagRleStr) != 0;
    bool use_dict = (flags & kFlagDictStr) != 0;
    bool use_bitpack = (flags & kFlagBitPackStr) != 0;

    if (use_rle) {
        return DecodeRle<std::string>(input, offset, size, use_bitpack);
    }
    if (use_dict) {
        return DecodeDict<std::string>(input, offset, size, use_bitpack);
    }

    std::vector<std::string> output;
    output.reserve(size);
    for (size_t i = 0; i < size; i++) {
        size_t str_size;
        std::memcpy(&str_size, input.data() + offset, sizeof(size_t));
        offset += sizeof(size_t);
        output.emplace_back(input.data() + offset, str_size);
        offset += str_size;
    }
    return output;
}

std::vector<char> EncodeColumn(std::shared_ptr<Column> column, Type type, size_t rows_number, size_t flags) {
    std::vector<char> encoded;
    bool use_lz4 = (flags & kFlagLz4) != 0;

    switch (type) {
        case Type::Int16: {
            const std::vector<int16_t>& column_data = static_cast<const ColumnTyped<int16_t>&>(*column).GetData();
            encoded = EncodeIntegerVector<int16_t>(column_data, flags);
            break;
        }
        case Type::Int32: {
            const std::vector<int32_t>& column_data = static_cast<const ColumnTyped<int32_t>&>(*column).GetData();
            encoded = EncodeIntegerVector<int32_t>(column_data, flags);
            break;
        }
        case Type::Int64: {
            const std::vector<int64_t>& column_data = static_cast<const ColumnTyped<int64_t>&>(*column).GetData();
            encoded = EncodeIntegerVector<int64_t>(column_data, flags);
            break;
        }
        case Type::Float: {
            const std::vector<float>& column_data = static_cast<const ColumnTyped<float>&>(*column).GetData();
            encoded = EncodeFloatVector<float>(column_data, flags);
            break;
        }
        case Type::Double: {
            const std::vector<double>& column_data = static_cast<const ColumnTyped<double>&>(*column).GetData();
            encoded = EncodeFloatVector<double>(column_data, flags);
            break;
        }
        case Type::Date: {
            const std::vector<Date>& column_data = static_cast<const ColumnTyped<Date>&>(*column).GetData();
            std::vector<int32_t> raw(rows_number);
            for (size_t i = 0; i < rows_number; i++) {
                raw[i] = column_data[i].GetValue();
            }
            encoded = EncodeIntegerVector<int32_t>(raw, flags);
            break;
        }
        case Type::Timestamp: {
            const std::vector<Timestamp>& column_data = static_cast<const ColumnTyped<Timestamp>&>(*column).GetData();
            std::vector<int64_t> raw(rows_number);
            for (size_t i = 0; i < rows_number; i++) {
                raw[i] = column_data[i].GetValue();
            }
            encoded = EncodeIntegerVector<int64_t>(raw, flags);
            break;
        }
        case Type::Char: {
            const std::vector<char>& column_data = static_cast<const ColumnTyped<char>&>(*column).GetData();
            encoded = EncodeIntegerVector<char>(column_data, flags);
            break;
        }
        case Type::String: {
            const std::vector<std::string>& column_data = static_cast<const ColumnTyped<std::string>&>(*column).GetData();
            encoded = EncodeStringVector(column_data, flags);
            break;
        }
        default:
            throw std::runtime_error("Unsupported type :: EncodeColumn");
    }

    if (use_lz4) {
        return Lz4Compress(encoded);
    }
    return encoded;
}

std::shared_ptr<Column> DecodeColumn(std::vector<char>& input, Type type, size_t rows_number, size_t flags) {
    bool used_lz4 = (flags & kFlagLz4) != 0;
    if (used_lz4) {
        input = Lz4Decompress(input);
    }

    switch (type) {
        case Type::Int16: {
            std::vector<int16_t> column_data = DecodeIntegerVector<int16_t>(input, rows_number, flags);
            return std::make_shared<ColumnTyped<int16_t>>(std::move(column_data));
        }
        case Type::Int32: {
            std::vector<int32_t> column_data = DecodeIntegerVector<int32_t>(input, rows_number, flags);
            return std::make_shared<ColumnTyped<int32_t>>(std::move(column_data));
        }
        case Type::Int64: {
            std::vector<int64_t> column_data = DecodeIntegerVector<int64_t>(input, rows_number, flags);
            return std::make_shared<ColumnTyped<int64_t>>(std::move(column_data));
        }
        case Type::Float: {
            std::vector<float> column_data = DecodeFloatVector<float>(input, rows_number, flags);
            return std::make_shared<ColumnTyped<float>>(std::move(column_data));
        }
        case Type::Double: {
            std::vector<double> column_data = DecodeFloatVector<double>(input, rows_number, flags);
            return std::make_shared<ColumnTyped<double>>(std::move(column_data));
        }
        case Type::Date: {
            std::vector<int32_t> raw = DecodeIntegerVector<int32_t>(input, rows_number, flags);
            std::vector<Date> column_data(rows_number);
            for (size_t i = 0; i < rows_number; i++) {
                column_data[i] = Date{raw[i]};
            }
            return std::make_shared<ColumnTyped<Date>>(std::move(column_data));
        }
        case Type::Timestamp: {
            std::vector<int64_t> raw = DecodeIntegerVector<int64_t>(input, rows_number, flags);
            std::vector<Timestamp> column_data(rows_number);
            for (size_t i = 0; i < rows_number; i++) {
                column_data[i] = Timestamp{raw[i]};
            }
            return std::make_shared<ColumnTyped<Timestamp>>(std::move(column_data));
        }
        case Type::Char: {
            std::vector<char> column_data = DecodeIntegerVector<char>(input, rows_number, flags);
            return std::make_shared<ColumnTyped<char>>(std::move(column_data));
        }
        case Type::String: {
            std::vector<std::string> column_data = DecodeStringVector(input, rows_number, flags);
            return std::make_shared<ColumnTyped<std::string>>(std::move(column_data));
        }
        default:
            throw std::runtime_error("Unsupported type :: DecodeColumn");
    }
}
