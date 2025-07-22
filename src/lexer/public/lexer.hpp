#pragma once
#include <string>
#include <vector>
#include <optional>

enum class TokenType {
    KeywordPrint,
    KeywordReturn,
    KeywordAsm,
    IntegerLiteral,
    Semicolon,
    RightParen,
    LeftParen,
    Quote,
    String,
    Comment,
};

struct Token {
    TokenType type;
    std::optional<std::string> value;
};

std::vector<Token> tokenize(const std::string& input);
