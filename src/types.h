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
inline std::string FromString<std::string>(std::string str) {
    return str;
}

template<>
inline std::string ToString<std::string>(std::string value) {
    return value;
}

template<>
inline Type FromString<Type>(std::string str) {
    if (str == "int16") {
        return Type::Int16;
    }
    if (str == "int32") {
        return Type::Int32;
    }
    if (str == "int64") {
        return Type::Int64;
    }
    if (str == "int128") {
        return Type::Int128;
    }
    if (str == "float") {
        return Type::Float;
    }
    if (str == "double") {
        return Type::Double;
    }
    if (str == "date") {
        return Type::Date;
    }
    if (str == "timestamp") {
        return Type::Timestamp;
    }
    if (str == "char") {
        return Type::Char;
    }
    if (str == "string") {
        return Type::String;
    }
    throw std::runtime_error("Invalid type :: FromString<Type>");
}

template<>
inline std::string ToString<Type>(Type value) {
    if (value == Type::Int16) {
        return "int16";
    }
    if (value == Type::Int32) {
        return "int32";
    }
    if (value == Type::Int64) {
        return "int64";
    }
    if (value == Type::Int128) {
        return "int128";
    }
    if (value == Type::Float) {
        return "float";
    }
    if (value == Type::Double) {
        return "double";
    }
    if (value == Type::Date) {
        return "date";
    }
    if (value == Type::Timestamp) {
        return "timestamp";
    }
    if (value == Type::Char) {
        return "char";
    }
    if (value == Type::String) {
        return "string";
    }
    throw std::runtime_error("Invalid type :: ToString<Type>");
}

template<>
inline Date FromString<Date>(std::string str) {
    using namespace std::chrono;
    int y, m, d;
    std::sscanf(str.c_str(), "%d-%d-%d", &y, &m, &d);
    year_month_day ymd = year{y} / month{static_cast<unsigned>(m)} / day{static_cast<unsigned>(d)};
    sys_days tp = sys_days{ymd};
    return Date{static_cast<int32_t>(tp.time_since_epoch().count())};
}

template<>
inline std::string ToString<Date>(Date value) {
    using namespace std::chrono;
    year_month_day ymd = year_month_day{sys_days{days{value.GetValue()}}};
    char buf[11];
    std::snprintf(buf, sizeof(buf), "%04d-%02u-%02u",
                  static_cast<int>(ymd.year()),
                  static_cast<unsigned>(ymd.month()),
                  static_cast<unsigned>(ymd.day()));
    return std::string(buf);
}

template<>
inline Timestamp FromString<Timestamp>(std::string str) {
    using namespace std::chrono;
    int y, mo, d, h, mi, s;
    std::sscanf(str.c_str(), "%d-%d-%d %d:%d:%d", &y, &mo, &d, &h, &mi, &s);
    year_month_day ymd = year{y} / month{static_cast<unsigned>(mo)} / day{static_cast<unsigned>(d)};
    sys_time<seconds> tp = sys_days{ymd} + hours{h} + minutes{mi} + seconds{s};
    return Timestamp{tp.time_since_epoch().count()};
}

template<>
inline std::string ToString<Timestamp>(Timestamp value) {
    using namespace std::chrono;
    sys_time<seconds> tp = sys_time<seconds>{seconds{value.GetValue()}};
    sys_days dp = floor<days>(tp);
    year_month_day ymd = year_month_day{dp};
    hh_mm_ss<seconds> hms = hh_mm_ss<seconds>{tp - dp};
    char buf[20];
    std::snprintf(buf, sizeof(buf), "%04d-%02u-%02u %02lld:%02lld:%02lld",
                  static_cast<int>(ymd.year()),
                  static_cast<unsigned>(ymd.month()),
                  static_cast<unsigned>(ymd.day()),
                  static_cast<long long>(hms.hours().count()),
                  static_cast<long long>(hms.minutes().count()),
                  static_cast<long long>(hms.seconds().count()));
    return std::string(buf);
}
