#pragma once

#include "computation_dag.hpp"
#include "dag_types.hpp"
#include "lexer.hpp"
#include "operation_registry.hpp"
#include <unordered_map>

class Parser {
public:
    Parser(
        const std::vector<Token>& tokens,
        const OperationRegistry& registry
    )
    : tokens_(tokens), dag_(registry)
    {
        // temprary solution
        type_map["VecF64"] = TypeId::VecF64;
        type_map["f64"] = TypeId::f64;
    };

    ComputationDAG& parse();

private:
    const Token& peek() const;
    const Token& next() const;
    const Token& prev() const;
    const Token& advance();
    const Token& expect(TokenType);

    void parse_statement();

    void parse_output();
    void parse_input(const std::string& var_name);
    void parse_assignment(const std::string& var_name);

    bool match(TokenType);

    size_t pos_ = 0;
    const std::vector<Token>& tokens_;
    ComputationDAG dag_;

    std::unordered_map<std::string, TypeId> type_map;
    std::unordered_map<std::string, NodeId> identifier_node;
};
