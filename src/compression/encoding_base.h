#pragma once

#include <cstring>
#include <string>
#include <vector>

template<typename T>
size_t ElementValueSize(const T&) {
    return sizeof(T);
}

template<>
size_t ElementValueSize<std::string>(const std::string& value);

template<typename T>
size_t EstimateRaw(const std::vector<T>& input) {
    size_t size = input.size() * sizeof(T);
    return size;
}

template<>
size_t EstimateRaw<std::string>(const std::vector<std::string>& input);

template<typename T>
void WriteValues(std::vector<char>& output, const std::vector<T>& input) {
    const char* ptr = reinterpret_cast<const char*>(input.data());
    output.insert(output.end(), ptr, ptr + input.size() * sizeof(T));
}

template<>
void WriteValues<std::string>(std::vector<char>& output, const std::vector<std::string>& input);

template<typename T>
std::vector<T> ReadValues(const std::vector<char>& input, size_t& offset, size_t count) {
    std::vector<T> output(count);
    std::memcpy(output.data(), input.data() + offset, count * sizeof(T));
    offset += count * sizeof(T);
    return output;
}

template<>
std::vector<std::string> ReadValues<std::string>(const std::vector<char>& input, size_t& offset, size_t count);
