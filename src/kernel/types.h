#pragma once

#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>

enum class Type : uint8_t {
    Int16,
    Int32,
    Int64,
    Int128,
    Float,
    Double,
    Date,
    Timestamp,
    Char,
    String,
};

class Date {
public:
    Date();
    explicit Date(int32_t d);
    int32_t GetValue() const;

private:
    int32_t days_;
};

bool operator==(const Date& a, const Date& b);
bool operator!=(const Date& a, const Date& b);
bool operator<(const Date& a, const Date& b);
bool operator>(const Date& a, const Date& b);
bool operator<=(const Date& a, const Date& b);
bool operator>=(const Date& a, const Date& b);

class Timestamp {
public:
    Timestamp();
    explicit Timestamp(int64_t s);
    int64_t GetValue() const;

private:
    int64_t seconds_;
};

bool operator==(const Timestamp& a, const Timestamp& b);
bool operator!=(const Timestamp& a, const Timestamp& b);
bool operator<(const Timestamp& a, const Timestamp& b);
bool operator>(const Timestamp& a, const Timestamp& b);
bool operator<=(const Timestamp& a, const Timestamp& b);
bool operator>=(const Timestamp& a, const Timestamp& b);

template<typename T>
T FromString(std::string str) {
    std::stringstream ss;
    ss << str;
    T value;
    ss >> value;
    return value;
}

template<typename T>
std::string ToString(T value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

template<>
Type FromString<Type>(std::string str);

template<>
std::string ToString<Type>(Type value);

template<>
std::string FromString<std::string>(std::string str);

template<>
std::string ToString<std::string>(std::string value);

template<>
Date FromString<Date>(std::string str);

template<>
std::string ToString<Date>(Date value);

template<>
Timestamp FromString<Timestamp>(std::string str);

template<>
std::string ToString<Timestamp>(Timestamp value);

template<typename T>
Type TypeOf();

template<>
Type TypeOf<int16_t>();

template<>
Type TypeOf<int32_t>();

template<>
Type TypeOf<int64_t>();

template<>
Type TypeOf<float>();

template<>
Type TypeOf<double>();

template<>
Type TypeOf<Date>();

template<>
Type TypeOf<Timestamp>();

template<>
Type TypeOf<char>();

template<>
Type TypeOf<std::string>();
