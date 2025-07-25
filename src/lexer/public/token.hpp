#pragma once
#include <string>
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
    KeywordFor,
    
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

    // Char
    Comma,
    
    // Special
    EndOfFile
};

struct Token {
    TokenType type;
    std::optional<std::string> value;
};

// Helper function to convert TokenType to string for debugging
std::string tokenTypeToString(TokenType type);
