#include "parser_base.hpp"
#include <iostream>
#include <cstdlib>
#include <charconv>

Parser::Parser(const std::vector<Token>& tokens, const std::string& file_path)
    : tokens(tokens), current(0), size(tokens.size()), current_file_path(file_path) {}

int Parser::parseInteger(std::string_view str) const {
    int value;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
    if (ec != std::errc{}) {
        reportError("Invalid integer format: '" + std::string(str) + "'. Expected a valid number like 42 or -123.");
    }
    return value;
}

std::string Parser::getErrorContext() const {
    std::string context = "at position " + std::to_string(current);
    
    if (current < size) {
        const auto& token = tokens[current];
        context += " (found " + tokenTypeToString(token.type);
        
        if (token.value.has_value() && !token.value.value().empty()) {
            context += ": '" + token.value.value() + "'";
        }
        context += ")";
    } else {
        context += " (end of input)";
    }
    
    return context;
}

void Parser::skipToRecoveryPoint() {
    while (hasTokens()) {
        const auto& token = peekToken();
        if (token.type == TokenType::Semicolon ||
            token.type == TokenType::RightBrace ||
            token.type == TokenType::KeywordReturn ||
            token.type == TokenType::KeywordPrint ||
            token.type == TokenType::KeywordLet ||
            token.type == TokenType::KeywordIf ||
            token.type == TokenType::KeywordWhile ||
            token.type == TokenType::Function ||
            token.type == TokenType::KeywordAsm) {
            break;
        }
        advance();
    }
}

std::string Parser::getSuggestions() const {
    if (!hasTokens()) return "";
    
    const auto& token = peekToken();
    std::string suggestions;
    
    if (current > 0) {
        const auto& prevToken = tokens[current - 1];
        
        if (prevToken.type == TokenType::KeywordLet) {
            suggestions = "\nSuggestions:\n   • let variableName;\n   • let x = 42;\n   • let name = \"value\";";
        }
        else if (prevToken.type == TokenType::KeywordPrint) {
            suggestions = "\nSuggestions:\n   • print(\"Hello\");\n   • print(42);\n   • print(variableName);";
        }
        else if (prevToken.type == TokenType::KeywordIf) {
            suggestions = "\nSuggestions:\n   • if (x == 5) { ... }\n   • if (name != \"test\") { ... }";
        }
        else if (prevToken.type == TokenType::Identifier && token.type != TokenType::LeftParen) {
            suggestions = "\nDid you mean:\n   • " + prevToken.value.value_or("name") + "(); (function call)\n   • let " + prevToken.value.value_or("name") + " = value; (assignment)";
        }
    }
    
    return suggestions;
}

void Parser::reportError(const std::string& message) const {
    std::cerr << "\nParse Error " << getErrorContext() << ":\n";
    std::cerr << "   " << message << "\n";
    
    std::string suggestions = getSuggestions();
    if (!suggestions.empty()) {
        std::cerr << suggestions << "\n";
    }
    
    if (current > 0 && current < size) {
        std::cerr << "\nContext:\n";
        
        if (current > 0) {
            const auto& prevToken = tokens[current - 1];
            std::cerr << "   Previous: " << tokenTypeToString(prevToken.type);
            if (prevToken.value.has_value()) {
                std::cerr << " '" << prevToken.value.value() << "'";
            }
            std::cerr << "\n";
        }
        
        const auto& currToken = tokens[current];
        std::cerr << "   Current:  " << tokenTypeToString(currToken.type);
        if (currToken.value.has_value()) {
            std::cerr << " '" << currToken.value.value() << "'";
        }
        std::cerr << " ← ERROR HERE\n";
        
        if (current + 1 < size) {
            const auto& nextToken = tokens[current + 1];
            std::cerr << "   Next:     " << tokenTypeToString(nextToken.type);
            if (nextToken.value.has_value()) {
                std::cerr << " '" << nextToken.value.value() << "'";
            }
            std::cerr << "\n";
        }
    }
    
    std::cerr << std::endl;
    std::exit(1);
}

void Parser::reportErrorWithRecovery(const std::string& message) {
    std::cerr << "\nParse Warning " << getErrorContext() << ":\n";
    std::cerr << "   " << message << "\n";
    
    std::string suggestions = getSuggestions();
    if (!suggestions.empty()) {
        std::cerr << suggestions << "\n";
    }
    
    std::cerr << "   Attempting to recover...\n" << std::endl;
    skipToRecoveryPoint();
}

void Parser::reportExpectedError(TokenType expected, const std::string& context) const {
    std::string message = "Expected " + tokenTypeToString(expected);
    
    if (!context.empty()) {
        message += " " + context;
    }
    
    if (current < size) {
        message += ", but found " + tokenTypeToString(tokens[current].type);
        if (tokens[current].value.has_value()) {
            message += " '" + tokens[current].value.value() + "'";
        }
    } else {
        message += ", but reached end of input";
    }
    
    if (expected == TokenType::Semicolon) {
        message += "\nRemember: All statements must end with a semicolon ';'";
    } else if (expected == TokenType::RightBrace) {
        message += "\nMake sure all '{' braces have matching '}' braces";
    } else if (expected == TokenType::RightParen) {
        message += "\nMake sure all '(' parentheses have matching ')' parentheses";
    } else if (expected == TokenType::LeftParen && current < size && tokens[current].type == TokenType::String) {
        message += "\nDid you forget parentheses? Use: print(\"text\") not print \"text\"";
    }
    
    reportError(message);
}

bool Parser::hasTokens(size_t count) const {
    return current + count <= size;
}

const Token& Parser::peekToken(size_t offset) const {
    if (current + offset >= size) {
        reportError("Unexpected end of input. Expected more tokens to complete the statement.");
    }
    return tokens[current + offset];
}

void Parser::advance(size_t count) {
    current += count;
}

void Parser::expectToken(TokenType expected, const std::string& context) {
    if (!hasTokens() || peekToken().type != expected) {
        reportExpectedError(expected, context);
    }
    advance();
}

void Parser::parseStatementList(AST& ast) {
    while (hasTokens() && peekToken().type != TokenType::RightBrace) {
        parseStatement(ast);
    }
}

void Parser::parseStatement(AST& ast) {
    if (!hasTokens()) {
        reportError("Unexpected end of input while parsing statement.");
    }

    const auto& token = peekToken();
    
    switch (token.type) {
        case TokenType::KeywordReturn:
            parseReturnStatement(ast);
            break;
        case TokenType::KeywordPrint:
            parsePrintStatement(ast);
            break;
        case TokenType::KeywordAsm:
            parseAsmStatement(ast);
            break;
        case TokenType::KeywordPub:
        case TokenType::Function:
            parseFunctionDefinition(ast);
            break;
        case TokenType::Comment:
            advance();
            break;
        case TokenType::MultilineComment:
            parseMultiComment(ast);
            break;
        case TokenType::Identifier:
            if (hasTokens(2) && peekToken(1).type == TokenType::LeftParen) {
                parseFunctionCall(ast);
            } else {
                std::string suggestion = "Did you mean to:\n"
                                       "   • Call a function: " + token.value.value_or("name") + "();\n"
                                       "   • Declare a variable: let " + token.value.value_or("name") + ";\n"
                                       "   • Assign to a variable: let " + token.value.value_or("name") + " = value;";
                reportError("Unexpected identifier '" + token.value.value_or("unknown") + "'.\n   " + suggestion);
            }
            break;
        case TokenType::KeywordLet:
            parseVariables(ast);
            break;
        case TokenType::KeywordIf:
            parseIfStatement(ast);
            break;
        case TokenType::KeywordWhile:
            parseWhile(ast);
            break;
        case TokenType::KeywordFor:
            parseFor(ast);
            break;
        case TokenType::KeywordMod:
            parseModStatement(ast);
            break;
        case TokenType::KeywordPrintln:
            parsePrintlnStatement(ast);
            break;
        default:
            reportError("Unexpected " + tokenTypeToString(token.type) + " at start of statement.\n"
                       "   Expected one of: 'return', 'print', 'println', 'fn', 'let', 'asm', 'if', 'while', 'mod' or identifier");
            advance();
            break;
    }
}

AST Parser::parse() {
    AST ast;
    ast.reserve(tokens.size() / 3);

    try {
        while (hasTokens() && peekToken().type != TokenType::EndOfFile) {
            parseStatement(ast);
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal parsing error: " << e.what() << std::endl;
        std::exit(1);
    }

    return ast;
}

AST parse(const std::vector<Token>& tokens, const std::string& file_path) {
    if (tokens.empty()) {
        std::cout << "Note: Input is empty, returning empty AST\n";
        return AST{};
    }

    Parser parser(tokens, file_path);
    return parser.parse();
}
