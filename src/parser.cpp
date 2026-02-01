#include "parser.hpp"
#include "computation_dag.hpp"
#include "lexer.hpp"
#include <stdexcept>
#include <string>

ComputationDAG& Parser::parse() {
    while (peek().type != TokenType::End) {
        parse_statement();
    }

    return dag_;
}

void Parser::parse_statement() {
    if (peek().type == TokenType::NewLine) {
        advance();
        return;
    }

    if (
        peek().type == TokenType::Identifier &&
        peek().lexeme == "return"
    ) {
        advance();
        parse_output();
        return;
    }

    std::string name = expect(TokenType::Identifier).lexeme.value();
    if (peek().type == TokenType::Colon) {
        parse_input(name);
        return;
    }

    if (peek().type == TokenType::Equals) {
        parse_assignment(name);
        return;
    }

    throw std::runtime_error("invalid statement");
}

void Parser::parse_output() {
    std::vector<NodeId> outputs;
    while (true) {
        const std::string output_var = expect(TokenType::Identifier).lexeme.value();
        auto it = identifier_node.find(output_var);
        if (it == identifier_node.end()) {
            throw std::runtime_error("undefined reference to output variable " + output_var);
        }

        outputs.push_back(it->second);
        if (peek().type != TokenType::NewLine) {
            expect(TokenType::Comma);
        } else break;
    }

    expect(TokenType::NewLine);
    dag_.set_outputs(std::move(outputs));
}

void Parser::parse_input(const std::string& var_name) {
    if (identifier_node.find(var_name) != identifier_node.end()) {
        throw std::runtime_error("redefinition of input variable " + var_name);
    }

    expect(TokenType::Colon);
    const std::string type_name = expect(TokenType::Identifier).lexeme.value();
    expect(TokenType::NewLine);

    auto it = type_map.find(type_name);
    if (it == type_map.end()) {
        throw std::runtime_error("unknown type: " + type_name);
    }

    NodeId id = dag_.add_input(it->second);
    identifier_node[var_name] = id;
}

void Parser::parse_assignment(const std::string& var_name) {
    if (identifier_node.find(var_name) != identifier_node.end()) {
        throw std::runtime_error("redefinition of variable " + var_name);
    }

    expect(TokenType::Equals);
    const std::string func_name = expect(TokenType::Identifier).lexeme.value();
    expect(TokenType::LParen);

    std::vector<NodeId> input_params{};

    if (peek().type != TokenType::RParen) {
        while (true) {
            const std::string param = expect(TokenType::Identifier).lexeme.value();
            auto it = identifier_node.find(param);
            if (it == identifier_node.end()) {
                throw std::runtime_error("undefined referece to variable " + param);
            }

            input_params.push_back(it->second);
            if (peek().type != TokenType::RParen) {
                expect(TokenType::Comma);
            } else {
                break;
            }
        }
    }

    expect(TokenType::RParen);
    expect(TokenType::NewLine);

    NodeId id = dag_.add_compute(func_name, std::move(input_params));
    identifier_node[var_name] = id;
}

const Token& Parser::prev() const {
    return tokens_[pos_ - 1];
}

const Token& Parser::peek() const {
    return tokens_[pos_];
}

const Token& Parser::next() const {
    return tokens_[pos_ + 1];
}

const Token& Parser::advance() {
    return tokens_[pos_++];
}

bool Parser::match(TokenType t) {
    if (peek().type == t) {
        advance();
        return true;
    }
    return false;
}

const char* token_type_name(TokenType t) {
    switch (t) {
        case TokenType::Identifier: return "Identifier";
        case TokenType::Equals:     return "=";
        case TokenType::LParen:     return "(";
        case TokenType::RParen:     return ")";
        case TokenType::Comma:      return ",";
        case TokenType::Colon:      return ":";
        case TokenType::NewLine:    return "NewLine";
        case TokenType::End:        return "End";
    }
    return "<unknown>";
}

const Token& Parser::expect(TokenType t) {
    const Token& tok = peek();

    if (!match(t)) {
        std::string msg = "unexpected token: ";
        msg += token_type_name(tok.type);

        if (tok.lexeme) {
            msg += " ('" + *tok.lexeme + "')";
        }

        msg += ", expected ";
        msg += token_type_name(t);
        msg += " at position " + std::to_string(pos_);

        throw std::runtime_error(msg);
    }
    return tok;
}

