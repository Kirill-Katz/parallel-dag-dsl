#include "computation_dag.hpp"

#include <queue>
#include <stdexcept>
#include <future>

template <>
bool ComputationDAG::type_matches<double>(TypeId t) {
    return t == TypeId::f64;
}

template <>
bool ComputationDAG::type_matches<std::vector<double>>(TypeId t) {
    return t == TypeId::VecF64;
}

NodeId ComputationDAG::add_input(TypeId input_type) {
    NodeId id = static_cast<NodeId>(dag.size());

    dag.push_back(Node {
        .op = nullptr,
        .inputs = {},
        .type = input_type,
        .storage = nullptr,
    });

    input_nodes.push_back(id);
    return id;
}

void ComputationDAG::bind_inputs(const std::vector<void*>& inputs_to_bind) {
    if (inputs_to_bind.size() != input_nodes.size()) {
        throw std::runtime_error("input count mismatch");
    }

    for (size_t i = 0; i < input_nodes.size(); ++i) {
        dag[input_nodes[i]].storage = inputs_to_bind[i];
    }
}

NodeId ComputationDAG::add_compute(const std::string& op_name, std::vector<NodeId> inputs) {
    NodeId id = static_cast<NodeId>(dag.size());

    const Operation* op = registry.find_op(op_name);

    dag.push_back(Node {
        .op = op,
        .inputs = std::move(inputs),
        .type = op->signature.output_type,
        .storage = allocate(op->signature.output_type)
    });

    return id;
}

void* ComputationDAG::allocate(TypeId t) {
    switch (t) {
        case TypeId::VecF64:
            return new std::vector<double>();
        case TypeId::f64:
            return new double();
    }

    throw std::runtime_error("Type now known by the allocator!");
}

void ComputationDAG::topological_run() {
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

void ComputationDAG::topological_run_parallel() {
    const size_t n = dag.size();

    std::vector<size_t> indeg(n, 0);
    std::vector<std::vector<size_t>> adj(n);

    for (size_t i = 0; i < n; ++i) {
        for (size_t dep : dag[i].inputs) {
            adj[dep].push_back(i);
            ++indeg[i];
        }
    }

    std::vector<size_t> frontier;
    for (size_t i = 0; i < n; ++i) {
        if (indeg[i] == 0) {
            frontier.push_back(i);
        }
    }

    size_t executed = 0;

    while (!frontier.empty()) {
        std::vector<std::future<void>> tasks;
        tasks.reserve(frontier.size());

        for (size_t idx : frontier) {
            tasks.emplace_back(std::async(std::launch::async, [&, idx] {
                Node& node = dag[idx];
                if (node.op) {
                    void* args[8];
                    for (size_t i = 0; i < node.inputs.size(); ++i) {
                        args[i] = dag[node.inputs[i]].storage;
                    }
                    node.op->fn(args, node.storage);
                }
            }));
        }

        for (auto& f : tasks) {
            f.get();
        }

        std::vector<size_t> next;
        for (size_t idx : frontier) {
            ++executed;
            for (auto& child : adj[idx]) {
                if(--indeg[child] == 0) {
                    next.push_back(child);
                }
            }
        }

        frontier = std::move(next);
    }

    if (executed != n) {
        throw std::runtime_error("Failed to execute all the nodes!");
    }
}


