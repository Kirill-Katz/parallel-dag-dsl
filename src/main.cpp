#include <alloca.h>
#include <iostream>
#include <numeric>
#include <vector>
#include <x86intrin.h>
#include <fstream>
#include <sstream>

#include "lexer.hpp"
#include "parser.hpp"
#include "operation_registry.hpp"
#include "computation_dag.hpp"

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

std::string read_file(const std::string& path) {
    std::ifstream in(path, std::ios::in | std::ios::binary);
    if (!in) {
        throw std::runtime_error("failed to open file");
    }

    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static inline uint64_t rdtscp_serialized() {
    unsigned aux;
    uint64_t t = __rdtscp(&aux);
    return t;
}

int main() {
    OperationRegistry registry;
    registry.register_op("SumVec", Operation {
        "SumVec",
        OpSignature {
            {TypeId::VecF64},
            TypeId::f64
        },
        SumVec
    });

    registry.register_op("SumVals", Operation {
        "SumVals",
        OpSignature {
            {TypeId::f64, TypeId::f64},
            TypeId::f64
        },
        SumVals
    });

    std::vector<double> input_vec1(10'000'000, 3.3);
    std::vector<double> input_vec2(10'000'000, 5.66);
    std::vector<double> input_vec3(10'000'000, 0.66);

    Lexer lexer;

    const std::string& source = read_file("../test.dsl");
    auto tokens = lexer.lex(source);
    Parser parser(tokens, registry);

    auto& dag = parser.parse();
    dag.bind_inputs({&input_vec1, &input_vec2, &input_vec3});

    uint64_t t0_par = rdtscp_serialized();
    dag.topological_run_parallel();
    uint64_t t1_par = rdtscp_serialized();
    uint64_t par_cycles = t1_par - t0_par;

    uint64_t t0_seq = rdtscp_serialized();
    dag.topological_run();
    uint64_t t1_seq = rdtscp_serialized();
    uint64_t seq_cycles = t1_seq - t0_seq;

    double speedup = double(seq_cycles) / double(par_cycles);

    std::cout << "Sequential cycles: " << seq_cycles << '\n';
    std::cout << "Parallel cycles:   " << par_cycles << '\n';
    std::cout << "Speedup:           " << speedup << "x\n";

    const auto& output_nodes = dag.get_output_nodes();
    for (NodeId id : output_nodes) {
        std::cout << "Output Node: " << id
                  << " -> " << dag.get_output<double>(id) << '\n';
    }

    return 0;
}
