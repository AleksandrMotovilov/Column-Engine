#include "src/kernel/encoding.h"
#include "src/kernel/types.h"

size_t GetCompressionFlags() {
    size_t flags = 0;
#ifdef ENABLE_RLE
    flags += kFlagRLE;
#endif
#ifdef ENABLE_DELTA
    flags += kFlagDelta;
#endif
#ifdef ENABLE_DICT
    flags += kFlagDict;
#endif
#ifdef ENABLE_LZ4
    flags += kFlagLZ4;
#endif
    return flags;
}

static std::vector<char> Lz4Compress(const std::vector<char>& input) {
    int uncompressed_size = static_cast<int>(input.size());
    int max_compressed_size = LZ4_compressBound(uncompressed_size);
    std::vector<char> output(2 * sizeof(int) + max_compressed_size);
    std::memcpy(output.data(), &uncompressed_size, sizeof(int));
    int compressed_size = LZ4_compress_default(
        input.data(),
        output.data() + 2 * sizeof(int),
        uncompressed_size,
        max_compressed_size
    );
    if (compressed_size == 0) {
        throw std::runtime_error("LZ4_compress_default failed");
    }
    std::memcpy(output.data() + sizeof(int), &compressed_size, sizeof(int));
    output.resize(2 * sizeof(int) + compressed_size);
    return output;
}

static std::vector<char> Lz4Decompress(const std::vector<char>& input) {
    int uncompressed_size;
    int compressed_size;
    std::memcpy(&uncompressed_size, input.data(), sizeof(int));
    std::memcpy(&compressed_size, input.data() + sizeof(int), sizeof(int));
    std::vector<char> output(uncompressed_size);
    int status = LZ4_decompress_safe(
        input.data() + 2 * sizeof(int),
        output.data(),
        compressed_size,
        uncompressed_size
    );
    if (status < uncompressed_size) {
        throw std::runtime_error("LZ4_decompress_safe failed");
    }
    return output;
}

template<typename T>
static std::vector<char> EncodeIntegerVector(const std::vector<T>& input, bool use_delta, bool use_rle) {
    size_t size = input.size();

    std::vector<T> working(size);
    
    if (use_delta && size > 1) {
        using U = std::make_unsigned_t<T>;
        working[0] = input[0];
        for (size_t i = 1; i < size; i++) {
            working[i] = static_cast<T>(static_cast<U>(input[i]) - static_cast<U>(input[i - 1]));
        }
    } else {
        working = input;
    }

    if (use_rle) {
        size_t run_count = 0;
        size_t i;
        i = 0;
        while (i < size) {
            size_t j = i + 1;
            while (j < size && working[j] == working[i]) {
                j++;
            }
            run_count++;
            i = j;
        }
        std::vector<char> output(sizeof(size_t) + run_count * (sizeof(T) + sizeof(size_t)));
        std::memcpy(output.data(), &run_count, sizeof(size_t));
        size_t offset = sizeof(size_t);
        i = 0;
        while (i < size) {
            size_t j = i + 1;
            while (j < size && working[j] == working[i]) {
                j++;
            }
            size_t count = j - i;
            std::memcpy(output.data() + offset, &working[i], sizeof(T));
            offset += sizeof(T);
            std::memcpy(output.data() + offset, &count, sizeof(size_t));
            offset += sizeof(size_t);
            i = j;
        }
        return output;
    }

    const char* raw = reinterpret_cast<const char*>(working.data());
    return std::vector<char>(raw, raw + size * sizeof(T));
}

template<typename T>
static std::vector<T> DecodeIntegerVector(const std::vector<char>& input, size_t size, bool used_delta, bool used_rle) {
    std::vector<T> working;
    working.reserve(size);

    if (used_rle) {
        size_t run_count;
        std::memcpy(&run_count, input.data(), sizeof(size_t));
        size_t offset = sizeof(size_t);
        for (size_t i = 0; i < run_count; i++) {
            T value;
            size_t count;
            std::memcpy(&value, input.data() + offset, sizeof(T));
            offset += sizeof(T);
            std::memcpy(&count, input.data() + offset, sizeof(size_t));
            offset += sizeof(size_t);
            for (size_t j = 0; j < count; j++) {
                working.push_back(value);
            }
        }
    } else {
        working.resize(size);
        std::memcpy(working.data(), input.data(), size * sizeof(T));
    }

    if (used_delta && size > 1) {
        using U = std::make_unsigned_t<T>;
        for (size_t i = 1; i < working.size(); i++) {
            working[i] = static_cast<T>(static_cast<U>(working[i]) + static_cast<U>(working[i - 1]));
        }
    }
    return working;
}

static std::vector<char> EncodeStringVector(const std::vector<std::string>& input, bool use_dict) {
    size_t size = input.size();

    if (!use_dict) {
        std::vector<char> output;
        for (const std::string& str : input) {
            size_t str_size = str.size();
            const char* ptr = reinterpret_cast<const char*>(&str_size);
            output.insert(output.end(), ptr, ptr + sizeof(size_t));
            output.insert(output.end(), str.begin(), str.end());
        }
        return output;
    }

    std::vector<std::string> dict;
    std::unordered_map<std::string, uint32_t> dict_map;
    std::vector<uint32_t> codes;
    codes.reserve(size);
    for (const std::string& str : input) {
        auto it = dict_map.find(str);
        if (it == dict_map.end()) {
            uint32_t index = static_cast<uint32_t>(dict.size());
            dict_map[str] = index;
            dict.push_back(str);
            codes.push_back(index);
        } else {
            codes.push_back(it->second);
        }
    }

    std::vector<char> output;
    size_t dict_size = dict.size();
    const char* ptr = reinterpret_cast<const char*>(&dict_size);
    output.insert(output.end(), ptr, ptr + sizeof(size_t));
    for (const std::string& str : dict) {
        size_t str_size = str.size();
        ptr = reinterpret_cast<const char*>(&str_size);
        output.insert(output.end(), ptr, ptr + sizeof(size_t));
        output.insert(output.end(), str.begin(), str.end());
    }
    ptr = reinterpret_cast<const char*>(codes.data());
    output.insert(output.end(), ptr, ptr + size * sizeof(uint32_t));
    return output;
}

static std::vector<std::string> DecodeStringVector(const std::vector<char>& input, size_t size, bool used_dict) {
    std::vector<std::string> output;
    output.reserve(size);

    if (!used_dict) {
        size_t offset = 0;
        for (size_t i = 0; i < size; i++) {
            size_t str_size;
            std::memcpy(&str_size, input.data() + offset, sizeof(size_t));
            offset += sizeof(size_t);
            output.emplace_back(reinterpret_cast<const char*>(input.data() + offset), str_size);
            offset += str_size;
        }
        return output;
    }

    size_t dict_size;
    std::memcpy(&dict_size, input.data(), sizeof(size_t));
    size_t offset = sizeof(size_t);
    std::vector<std::string> dict(dict_size);
    for (size_t i = 0; i < dict_size; i++) {
        size_t str_size;
        std::memcpy(&str_size, input.data() + offset, sizeof(size_t));
        offset += sizeof(size_t);
        dict[i].assign(reinterpret_cast<const char*>(input.data() + offset), str_size);
        offset += str_size;
    }
    for (size_t i = 0; i < size; i++) {
        uint32_t code;
        std::memcpy(&code, input.data() + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        output.push_back(dict[code]);
    }
    return output;
}

std::vector<char> EncodeColumn(std::shared_ptr<Column> column, Type type, size_t rows_number, size_t flags) {
    bool use_rle = (flags & kFlagRLE) != 0;
    bool use_delta = (flags & kFlagDelta) != 0;
    bool use_dict = (flags & kFlagDict) != 0;
    bool use_lz4 = (flags & kFlagLZ4) != 0;

    std::vector<char> encoded;

    switch (type) {
        case Type::Int16: {
            const std::vector<int16_t>& column_data = dynamic_cast<const ColumnTyped<int16_t>&>(*column).GetData();
            encoded = EncodeIntegerVector<int16_t>(column_data, use_delta, use_rle);
            break;
        }
        case Type::Int32: {
            const std::vector<int32_t>& column_data = dynamic_cast<const ColumnTyped<int32_t>&>(*column).GetData();
            encoded = EncodeIntegerVector<int32_t>(column_data, use_delta, use_rle);
            break;
        }
        case Type::Int64: {
            const std::vector<int64_t>& column_data = dynamic_cast<const ColumnTyped<int64_t>&>(*column).GetData();
            encoded = EncodeIntegerVector<int64_t>(column_data, use_delta, use_rle);
            break;
        }
        case Type::Float: {
            const std::vector<float>& column_data = dynamic_cast<const ColumnTyped<float>&>(*column).GetData();
            const char* raw = reinterpret_cast<const char*>(column_data.data());
            encoded = std::vector<char>(raw, raw + rows_number * sizeof(float));
            break;
        }
        case Type::Double: {
            const std::vector<double>& column_data = dynamic_cast<const ColumnTyped<double>&>(*column).GetData();
            const char* raw = reinterpret_cast<const char*>(column_data.data());
            encoded = std::vector<char>(raw, raw + rows_number * sizeof(double));
            break;
        }
        case Type::Date: {
            const std::vector<Date>& column_data = dynamic_cast<const ColumnTyped<Date>&>(*column).GetData();
            std::vector<int32_t> raw(rows_number);
            for (size_t i = 0; i < rows_number; i++) {
                raw[i] = column_data[i].GetValue();
            }
            encoded = EncodeIntegerVector<int32_t>(raw, use_delta, use_rle);
            break;
        }
        case Type::Timestamp: {
            const std::vector<Timestamp>& column_data = dynamic_cast<const ColumnTyped<Timestamp>&>(*column).GetData();
            std::vector<int64_t> raw(rows_number);
            for (size_t i = 0; i < rows_number; i++) {
                raw[i] = column_data[i].GetValue();
            }
            encoded = EncodeIntegerVector<int64_t>(raw, use_delta, use_rle);
            break;
        }
        case Type::Char: {
            const std::vector<char>& column_data = dynamic_cast<const ColumnTyped<char>&>(*column).GetData();
            encoded = EncodeIntegerVector<char>(column_data, use_delta, use_rle);
            break;
        }
        case Type::String: {
            const std::vector<std::string>& column_data = dynamic_cast<const ColumnTyped<std::string>&>(*column).GetData();
            encoded = EncodeStringVector(column_data, use_dict);
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

std::shared_ptr<Column> DecodeColumn(std::vector<char>& data, Type type, size_t rows_number, size_t flags) {
    bool used_rle = (flags & kFlagRLE) != 0;
    bool used_delta = (flags & kFlagDelta) != 0;
    bool used_dict = (flags & kFlagDict) != 0;
    bool used_lz4 = (flags & kFlagLZ4) != 0;

    std::vector<char> decompressed;

    if (used_lz4) {
        decompressed = Lz4Decompress(data);
        data = decompressed;
    }

    switch (type) {
        case Type::Int16: {
            std::vector<int16_t> column_data = DecodeIntegerVector<int16_t>(data, rows_number, used_delta, used_rle);
            return std::make_shared<ColumnTyped<int16_t>>(std::move(column_data));
        }
        case Type::Int32: {
            std::vector<int32_t> column_data = DecodeIntegerVector<int32_t>(data, rows_number, used_delta, used_rle);
            return std::make_shared<ColumnTyped<int32_t>>(std::move(column_data));
        }
        case Type::Int64: {
            std::vector<int64_t> column_data = DecodeIntegerVector<int64_t>(data, rows_number, used_delta, used_rle);
            return std::make_shared<ColumnTyped<int64_t>>(std::move(column_data));
        }
        case Type::Float: {
            std::vector<float> column_data(rows_number);
            std::memcpy(column_data.data(), data.data(), rows_number * sizeof(float));
            return std::make_shared<ColumnTyped<float>>(std::move(column_data));
        }
        case Type::Double: {
            std::vector<double> column_data(rows_number);
            std::memcpy(column_data.data(), data.data(), rows_number * sizeof(double));
            return std::make_shared<ColumnTyped<double>>(std::move(column_data));
        }
        case Type::Date: {
            std::vector<int32_t> raw = DecodeIntegerVector<int32_t>(data, rows_number, used_delta, used_rle);
            std::vector<Date> column_data(rows_number);
            for (size_t i = 0; i < rows_number; i++) {
                column_data[i] = Date{raw[i]};
            }
            return std::make_shared<ColumnTyped<Date>>(std::move(column_data));
        }
        case Type::Timestamp: {
            std::vector<int64_t> raw = DecodeIntegerVector<int64_t>(data, rows_number, used_delta, used_rle);
            std::vector<Timestamp> column_data(rows_number);
            for (size_t i = 0; i < rows_number; i++) {
                column_data[i] = Timestamp{raw[i]};
            }
            return std::make_shared<ColumnTyped<Timestamp>>(std::move(column_data));
        }
        case Type::Char: {
            std::vector<char> column_data = DecodeIntegerVector<char>(data, rows_number, used_delta, used_rle);
            return std::make_shared<ColumnTyped<char>>(std::move(column_data));
        }
        case Type::String: {
            std::vector<std::string> column_data = DecodeStringVector(data, rows_number, used_dict);
            return std::make_shared<ColumnTyped<std::string>>(std::move(column_data));
        }
        default:
            throw std::runtime_error("Unsupported type :: DecodeColumn");
    }
}
