#pragma once
#include <string>
#include <vector>
#include <optional>

enum class TokenType {
    Identifier,
    Equals,
    LParen,
    RParen,
    Comma,
    NewLine,
    Colon,
    End
};

struct Token {
    TokenType type;
    std::optional<std::string> lexeme;
};

class Lexer {
public:
    std::vector<Token> lex(const std::string& src);
    const char* token_to_str(TokenType t);
    void debug_log(const std::vector<Token>& tokens);
};

