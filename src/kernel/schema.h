#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include "src/kernel/types.h"

class Schema {
public:
    Schema(std::vector<std::string> names, std::vector<Type> types);
    size_t GetColumnsNumber() const;
    size_t FindColumn(const std::string& name) const;
    const std::string& GetName(size_t index) const;
    Type GetType(size_t index) const;
    std::vector<std::string> GetNames() const;
    std::vector<Type> GetTypes() const;

private:
    std::vector<std::string> names_;
    std::vector<Type> types_;
};
