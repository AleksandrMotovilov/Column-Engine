#include "src/kernel/encoding_base.h"

template<>
size_t ElementValueSize<std::string>(const std::string& value) {
    return sizeof(size_t) + value.size();
}

template<>
size_t EstimateRaw<std::string>(const std::vector<std::string>& input) {
    size_t size = 0;
    for (const std::string& value : input) {
        size += ElementValueSize<std::string>(value);
    }
    return size;
}

template<>
void WriteValues<std::string>(std::vector<char>& output, const std::vector<std::string>& input) {
    for (const std::string& str : input) {
        size_t size = str.size();
        const char* ptr = reinterpret_cast<const char*>(&size);
        output.insert(output.end(), ptr, ptr + sizeof(size_t));
        output.insert(output.end(), str.begin(), str.end());
    }
}

template<>
std::vector<std::string> ReadValues<std::string>(const std::vector<char>& input, size_t& offset, size_t count) {
    std::vector<std::string> output(count);
    for (size_t i = 0; i < count; i++) {
        size_t size;
        std::memcpy(&size, input.data() + offset, sizeof(size_t));
        offset += sizeof(size_t);
        output[i].assign(input.data() + offset, size);
        offset += size;
    }
    return output;
}
