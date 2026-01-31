#include <alloca.h>
#include <iostream>
#include <numeric>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <queue>

enum class TypeId {
    VectorF64,
    F64
};

struct OpSignature {
    std::vector<TypeId> input_types;
    TypeId output_types;
};

using OpFn = void(*)(void** inputs, void* output);
struct Operation {
    std::string name;
    OpSignature signature;
    OpFn fn;
};

std::unordered_map<std::string, Operation> registry;

void SumVec(void** inputs, void* output) {
    auto* in = static_cast<const std::vector<double>*>(inputs[0]);
    auto* out = static_cast<double*>(output);

    *out = std::accumulate(in->begin(), in->end(), 0.0);
}

void SumVals(void** inputs, void* output) {
    auto* val1 = static_cast<const double*>(inputs[0]);
    auto* val2 = static_cast<const double*>(inputs[1]);

    auto* out = static_cast<double*>(output);
    *out = *val1 + *val2;
}

using NodeId = uint32_t;
struct Node {
    const Operation* op;
    std::vector<NodeId> inputs;
    TypeId type;
    void* storage;
};

void register_ops() {
    auto vec_sum = Operation {
        "SumVec",
        OpSignature {
            {TypeId::VectorF64},
            TypeId::F64
        },
        SumVec
    };

    auto sum_vals = Operation {
        "SumVals",
        OpSignature {
            {TypeId::F64, TypeId::F64},
            TypeId::F64
        },
        SumVals
    };

    registry["SumVec"] = vec_sum;
    registry["SumVals"] = sum_vals;
}

void topological_run(std::vector<Node>& dag) {
    const size_t n = dag.size();

    std::vector<size_t> indeg(n, 0);
    std::vector<std::vector<size_t>> adj(n);

    for (size_t i = 0; i < n; ++i) {
        for (size_t dep : dag[i].inputs) {
            adj[dep].push_back(i);
            ++indeg[i];
        }
    }

    std::queue<size_t> bfs;
    for (size_t i = 0; i < n; ++i) {
        if (indeg[i] == 0) {
            bfs.push(i);
        }
    }

    size_t executed = 0;

    while (!bfs.empty()) {
        size_t idx = bfs.front();
        bfs.pop();

        Node& node = dag[idx];

        if (node.op) {
            void* args[8];
            for (size_t i = 0; i < node.inputs.size(); ++i) {
                args[i] = dag[node.inputs[i]].storage;
            }
            node.op->fn(args, node.storage);
        }

        ++executed;

        for (auto& child : adj[idx]) {
            if(--indeg[child] == 0) {
                bfs.push(child);
            }
        }
    }

    if (executed != n) {
        throw std::runtime_error("Failed to execute all the nodes!");
    }
}

void run(std::vector<Node>& dag) {
    for (Node& n : dag) {
        if (!n.op) continue;

        void* args[8];
        for (size_t i = 0; i < n.inputs.size(); ++i) {
            args[i] = dag[n.inputs[i]].storage;
        }

        n.op->fn(args, n.storage);
    }
}

void* allocate(TypeId t) {
    switch (t) {
        case TypeId::VectorF64:
            return new std::vector<double>();
        case TypeId::F64:
            return new double();
    }
    std::abort();
}

int main() {
    register_ops();

    std::vector<double> input_vec(10, 3.3);

    std::vector<Node> dag;

    auto input_node = Node {
        nullptr,
        {},
        TypeId::VectorF64,
        &input_vec
    };
    dag.push_back(input_node);

    Node sum_node {
        .op = &registry["SumVec"],
        .inputs = {0},
        .type = TypeId::F64,
        .storage = allocate(TypeId::F64)
    };
    dag.push_back(sum_node);

    Node summation_node {
        .op = &registry["SumVals"],
        .inputs = {1, 1},
        .type = TypeId::F64,
        .storage = allocate(TypeId::F64)
    };
    dag.push_back(summation_node);

    topological_run(dag);

    auto* result = static_cast<double*>(dag[2].storage);
    std::cout << "Result: " << *result << "\n";

    delete static_cast<double*>(dag[1].storage);
    delete static_cast<double*>(dag[2].storage);

    return 0;
}
