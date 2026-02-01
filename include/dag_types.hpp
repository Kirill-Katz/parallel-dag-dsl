#pragma once
#include <vector>
#include <string>
#include <cstdint>

enum class TypeId {
    VecF64,
    f64
};

struct OpSignature {
    std::vector<TypeId> input_types;
    TypeId output_type;
};

using OpFn = void(*)(void** inputs, void* output);
struct Operation {
    std::string name;
    OpSignature signature;

    OpFn fn;
};

using NodeId = uint32_t;
struct Node {
    const Operation* op;
    std::vector<NodeId> inputs;
    TypeId type;
    void* storage;
};
