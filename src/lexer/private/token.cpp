#include "../public/token.hpp"

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::KeywordPrint: return "'print'";
        case TokenType::KeywordPrintln: return "'println'";
        case TokenType::KeywordReturn: return "'return'";
        case TokenType::KeywordAsm: return "'asm'";
        case TokenType::KeywordPub: return "'pub'";
        case TokenType::KeywordLet: return "'let'";
        case TokenType::KeywordIf: return "'if'";
        case TokenType::KeywordElse: return "'else'";
        case TokenType::KeywordElif: return "'elif'";
        case TokenType::KeywordWhile: return "'while'";
        case TokenType::KeywordFor: return "'for'";
        case TokenType::KeywordMod: return "'mod'";
        case TokenType::U8: return "'u8'";
        case TokenType::U16: return "'u16'";
        case TokenType::U32: return "'u32'";
        case TokenType::U64: return "'u64'";
        case TokenType::I8: return "'i8'";
        case TokenType::I16: return "'i16'";
        case TokenType::I32: return "'i32'";
        case TokenType::I64: return "'i64'";
        case TokenType::IntegerLiteral: return "integer";
        case TokenType::HexLiteral: return "hex literal";
        case TokenType::BooleanLiteral: return "boolean";
        case TokenType::FloatLiteral: return "float";
        case TokenType::String: return "string";
        case TokenType::Identifier: return "identifier";
        case TokenType::Function: return "'fn'";
        case TokenType::Equal: return "'='";
        case TokenType::Plus: return "'+'";
        case TokenType::Minus: return "'-'";
        case TokenType::Multiply: return "'*'";
        case TokenType::Divide: return "'/'";
        case TokenType::Modulo: return "'%'";
        case TokenType::And: return "'&&'";
        case TokenType::Or: return "'||'";
        case TokenType::Not: return "'!'";
        case TokenType::EqualEqual: return "'=='";
        case TokenType::NotEqual: return "'!='";
        case TokenType::LessThan: return "'<'";
        case TokenType::LessThanEqual: return "'<='";
        case TokenType::GreaterThan: return "'>'";
        case TokenType::GreaterThanEqual: return "'>='";
        case TokenType::LeftParen: return "'('";
        case TokenType::RightParen: return "')'";
        case TokenType::LeftBrace: return "'{'";
        case TokenType::RightBrace: return "'}'";
        case TokenType::Semicolon: return "';'";
        case TokenType::Colon: return "':'";
        case TokenType::Quote: return "'\"'";
        case TokenType::Comment: return "comment";
        case TokenType::MultilineComment: return "multiline comment";
        case TokenType::EndMultilineComment: return "'*/'";
        case TokenType::Comma: return "','";
        case TokenType::EndOfFile: return "end of file";
        default: return "unknown token";
    }
}