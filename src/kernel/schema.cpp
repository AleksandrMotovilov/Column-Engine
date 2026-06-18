#include "src/kernel/schema.h"

Schema::Schema(std::vector<std::string> names, std::vector<Type> types) {
    if (names.size() != types.size()) {
        throw std::runtime_error("names and types size mismatch :: Schema");
    }
    names_ = std::move(names);
    types_ = std::move(types);
}

size_t Schema::GetColumnsNumber() const {
    return names_.size();
}

size_t Schema::FindColumn(const std::string& name) const {
    for (size_t i = 0; i < names_.size(); i++) {
        if (names_[i] == name) {
            return i;
        }
    }
    return names_.size();
}

const std::string& Schema::GetName(size_t index) const {
    return names_[index];
}

Type Schema::GetType(size_t index) const {
    return types_[index];
}

std::vector<std::string> Schema::GetNames() const {
    return names_;
}

std::vector<Type> Schema::GetTypes() const {
    return types_;
}
