#pragma once
#include <vector>
#include <string>
#include <stdexcept>

#include "dag_types.hpp"
#include "operation_registry.hpp"

class ComputationDAG {
public:
    ComputationDAG(const OperationRegistry& op_registry)
    : registry(op_registry)
    {};

    ComputationDAG(const ComputationDAG&) = delete;
    ComputationDAG& operator=(const ComputationDAG&) = delete;

    ComputationDAG(ComputationDAG&&) = delete;
    ComputationDAG& operator=(ComputationDAG&&) = delete;

    void topological_run();
    void topological_run_parallel();

    NodeId add_input(TypeId input_type);
    void bind_inputs(const std::vector<void*>& inputs_to_bind);

    NodeId add_compute(const std::string& op_name, std::vector<NodeId> inputs);

    void set_outputs(std::vector<NodeId> out) {
        output_nodes = std::move(out);
    }

    const std::vector<NodeId>& get_output_nodes() {
        return output_nodes;
    }

    template<typename T>
    T& get_output(NodeId id);

    template <typename T>
    bool type_matches(TypeId);

    ~ComputationDAG() {
        for (size_t i = 0; i < dag.size(); ++i) {
            if (dag[i].op) {
                delete static_cast<double*>(dag[i].storage);
            }
        }
    }

private:
    void* allocate(TypeId t); // this is bad and should be refactored
    const OperationRegistry& registry;
    std::vector<Node> dag;

    std::vector<NodeId> input_nodes;
    std::vector<NodeId> output_nodes;
};

template<typename T>
T& ComputationDAG::get_output(NodeId id) {
    Node& n = dag[id];

    if (!type_matches<T>(n.type)) {
        throw std::runtime_error("type mismatch in get_output");
    }

    return *static_cast<T*>(n.storage);
}



