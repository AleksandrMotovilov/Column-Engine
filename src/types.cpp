#include "src/types.h"

Date::Date() {
    days_ = 0;
}

Date::Date(int32_t d) {
    days_ = d;
}

int32_t Date::GetValue() const {
    return days_;
}

bool operator==(const Date& a, const Date& b) {
    return a.GetValue() == b.GetValue();
}

bool operator!=(const Date& a, const Date& b) {
    return a.GetValue() != b.GetValue();
}

bool operator<(const Date& a, const Date& b) {
    return a.GetValue() < b.GetValue();
}

bool operator>(const Date& a, const Date& b) {
    return a.GetValue() > b.GetValue();
}

bool operator<=(const Date& a, const Date& b) {
    return a.GetValue() <= b.GetValue();
}

bool operator>=(const Date& a, const Date& b) {
    return a.GetValue() >= b.GetValue();
}

Timestamp::Timestamp() {
    seconds_ = 0;
}

Timestamp::Timestamp(int64_t s) {
    seconds_ = s;
}

int64_t Timestamp::GetValue() const {
    return seconds_;
}

bool operator==(const Timestamp& a, const Timestamp& b) {
    return a.GetValue() == b.GetValue();
}

bool operator!=(const Timestamp& a, const Timestamp& b) {
    return a.GetValue() != b.GetValue();
}

bool operator<(const Timestamp& a, const Timestamp& b) {
    return a.GetValue() < b.GetValue();
}

bool operator>(const Timestamp& a, const Timestamp& b) {
    return a.GetValue() > b.GetValue();
}

bool operator<=(const Timestamp& a, const Timestamp& b) {
    return a.GetValue() <= b.GetValue();
}

bool operator>=(const Timestamp& a, const Timestamp& b) {
    return a.GetValue() >= b.GetValue();
}
