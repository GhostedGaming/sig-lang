#pragma once
#include <string>
#include <vector>
#include <optional>

enum class TokenType {
    KeywordPrint,
    KeywordReturn,
    IntegerLiteral,
    Semicolon,
};

struct Token {
    TokenType type;
    std::optional<std::string> value;
};

std::vector<Token> tokenize(const std::string& input);
