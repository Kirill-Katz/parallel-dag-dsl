#include "lexer.hpp"
#include <cctype>
#include <optional>
#include <stdexcept>
#include <iostream>

static inline bool is_letter(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

static inline bool is_alnum(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

const char* Lexer::token_to_str(TokenType t) {
    switch (t) {
        case TokenType::Identifier: return "Identifier";
        case TokenType::Equals:     return "Equals";
        case TokenType::LParen:     return "LParen";
        case TokenType::RParen:     return "RParen";
        case TokenType::Comma:      return "Comma";
        case TokenType::NewLine:    return "NewLine";
        case TokenType::Colon:      return "Colon";
        case TokenType::End:        return "End";
    }
    return "Unknown";
}

void Lexer::debug_log(const std::vector<Token>& tokens) {
    for (const Token& t : tokens) {
        std::cout << token_to_str(t.type);
        if (t.lexeme) {
            std::cout << "('" << *t.lexeme << "')";
        }
        std::cout << '\n';
    }
}

std::vector<Token> Lexer::lex(const std::string& src) {
    std::vector<Token> out;

    size_t i = 0;
    while (i < src.size()) {
        bool non_ident = false;

        if (src[i] == '\n') {
            out.push_back({TokenType::NewLine, std::nullopt});
            ++i; continue;
        }

        if (src[i] == '(') {
            out.push_back({TokenType::LParen, std::nullopt});
            ++i; continue;
        }

        if (src[i] == ')') {
            out.push_back({TokenType::RParen, std::nullopt});
            ++i; continue;
        }

        if (src[i] == ',') {
            out.push_back({TokenType::Comma, std::nullopt});
            ++i; continue;
        }

        if (src[i] == '=') {
            out.push_back({TokenType::Equals, std::nullopt});
            ++i; continue;
        }

        if (src[i] == ':') {
            out.push_back({TokenType::Colon, std::nullopt});
            ++i; continue;
        }

        if (std::isspace(static_cast<unsigned char>(src[i]))) {
            ++i;
            continue;
        }

        if (is_letter(src[i])) {
            std::string lexeme;
            lexeme.push_back(src[i]);
            ++i;

            while (i < src.size() && is_alnum(src[i])) {
                lexeme.push_back(src[i]);
                ++i;
            }

            out.push_back({TokenType::Identifier, std::move(lexeme)});
            continue;
        }

        throw std::runtime_error("unexpected character");
    }

    out.push_back({TokenType::NewLine, std::nullopt});
    out.push_back({TokenType::End, std::nullopt});
    return out;
}
