#pragma once
#include <unordered_map>
#include <string>
#include "dag_types.hpp"

class OperationRegistry {

public:
    void register_op(const std::string& external_name, Operation op);
    const Operation* find_op(const std::string& external_name) const;

private:
    std::unordered_map<std::string, Operation> registry;

};
