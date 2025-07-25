#include "../public/token.hpp"

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::KeywordPrint: return "'print'";
        case TokenType::KeywordReturn: return "'return'";
        case TokenType::KeywordAsm: return "'asm'";
        case TokenType::KeywordPub: return "'pub'";
        case TokenType::KeywordLet: return "'let'";
        case TokenType::KeywordIf: return "'if'";
        case TokenType::KeywordElse: return "'else'";
        case TokenType::KeywordElif: return "'elif'";
        case TokenType::KeywordWhile: return "'while'";
        case TokenType::KeywordFor: return "'for'";
        case TokenType::IntegerLiteral: return "integer";
        case TokenType::String: return "string";
        case TokenType::Identifier: return "identifier";
        case TokenType::Function: return "'fn'";
        case TokenType::Equal: return "'='";
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
        case TokenType::Quote: return "'\"'";
        case TokenType::Comment: return "comment";
        case TokenType::MultilineComment: return "multiline comment";
        case TokenType::EndMultilineComment: return "'*/'";
        case TokenType::Comma: return "','";
        case TokenType::EndOfFile: return "end of file";
        default: return "unknown token";
    }
}
