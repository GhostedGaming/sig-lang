#pragma once
#include <string>
#include <optional>

enum class TokenType {
    // Keywords
    KeywordPrint,
    KeywordPrintln,
    KeywordReturn,
    KeywordAsm,
    KeywordPub,
    KeywordLet,
    KeywordIf,
    KeywordElse,
    KeywordElif,
    KeywordWhile,
    KeywordFor,
    KeywordMod,
    KeywordStruct,
    
    // Type keywords
    U8,
    U16,
    U32,
    U64,
    I8,
    I16,
    I32,
    I64,
    
    // Literals
    IntegerLiteral,
    HexLiteral,
    BooleanLiteral,
    FloatLiteral,
    String,
    
    // Identifiers
    Identifier,
    Function,
    
    // Operators
    Equal,
    Plus,
    Minus,
    Multiply,
    Divide,
    Modulo,
    
    // Bitwise operators
    BitwiseAnd,    // &
    BitwiseOr,     // |
    BitwiseXor,    // ^
    LeftShift,     // <<
    RightShift,    // >>
    
    // Pointer operators  
    Asterisk,      // * (dereference and pointer type)
    AddressOf,     // & (address-of, now handled by BitwiseAnd)
    
    // Type casting
    KeywordAs,     // as (type casting)
    
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
    Colon,
    Dot,          // . (member access)
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
