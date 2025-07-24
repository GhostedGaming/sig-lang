#pragma once
#include <string>
#include <vector>
#include <optional>

enum class TokenType {
    // Keywords
    KeywordPrint,
    KeywordReturn,
    KeywordAsm,
    KeywordPub,
    KeywordLet,
    KeywordIf,
    KeywordElse,
    KeywordElif,
    KeywordWhile,
    
    // Literals
    IntegerLiteral,
    String,
    
    // Identifiers
    Identifier,
    Function,
    
    // Operators
    Equal,
    
    // Logical Operators
    And,           // &&
    Or,            // ||
    Not,           // !
    EqualEqual,    // ==
    NotEqual,      // !=
    LessThan,      // <
    LessThanEqual, // <=
    GreaterThan,   // >
    GreaterThanEqual, // >=
    
    // Delimiters
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    Semicolon,
    Quote,
    
    // Comments
    Comment,
    MultilineComment,
    EndMultilineComment,
    
    // Special
    EndOfFile
};

struct Token {
    TokenType type;
    std::optional<std::string> value;
};

// Function declaration
std::vector<Token> tokenize(const std::string& input);