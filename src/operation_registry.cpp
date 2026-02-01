#include "operation_registry.hpp"
#include "dag_types.hpp"

#include <string>
#include <stdexcept>

void OperationRegistry::register_op(const std::string& external_name, Operation op) {
    registry[external_name] = op;
}

const Operation* OperationRegistry::find_op(const std::string& op_name) const {
    auto it = registry.find(op_name);
    if (it == registry.end()) {
        throw std::runtime_error("unknown operation: " + op_name);
    }
    return &it->second;
}
