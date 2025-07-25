#include "parser_base.hpp"
#include <lexer/public/token.hpp>
#include <iostream>
#include <cstdlib>
#include <charconv>

ParserBase::ParserBase(const std::vector<Token>& tokens)
    : tokens(tokens), current(0), size(tokens.size()) {}

inline bool ParserBase::hasTokens(size_t count) const {
    return current + count <= size;
}

inline const Token& ParserBase::peekToken(size_t offset) const {
    if (current + offset >= size) {
        reportError("Unexpected end of input. Expected more tokens to complete the statement.");
    }
    return tokens[current + offset];
}

inline void ParserBase::advance(size_t count) {
    current += count;
}

inline void ParserBase::expectToken(TokenType expected, const std::string& context) {
    if (!hasTokens() || peekToken().type != expected) {
        reportExpectedError(expected, context);
    }
    advance();
}

std::string ParserBase::getErrorContext() const {
    std::string context = "at position " + std::to_string(current);
    
    if (current < size) {
        const auto& token = tokens[current];
        context += " (token: " + tokenTypeToString(token.type);
        if (token.value.has_value()) {
            context += " '" + token.value.value() + "'";
        }
        context += ")";
    } else {
        context += " (end of input)";
    }
    
    return context;
}

std::string ParserBase::getSuggestions() const {
    if (current >= size) {
        return "\nSuggestions:\n   • Check if you're missing a semicolon ';' at the end of the previous statement\n   • Ensure all braces '{' and '}' are properly matched";
    }
    
    const auto& token = tokens[current];
    std::string suggestions;
    
    // Context-aware suggestions based on current token
    if (token.type == TokenType::Identifier) {
        suggestions += "\nSuggestions:\n";
        suggestions += "   • If defining a function: fn " + token.value.value_or("name") + "() { ... }\n";
        suggestions += "   • If calling a function: " + token.value.value_or("name") + "();\n";
        suggestions += "   • If declaring a variable: let " + token.value.value_or("name") + " = value;\n";
        suggestions += "   • If this is a statement, it might need to be inside a function";
    } else if (token.type == TokenType::LeftBrace) {
        suggestions += "\nNote: Make sure this '{' has a matching '}'";
    } else if (token.type == TokenType::RightBrace) {
        suggestions += "\nNote: This '}' might be extra or missing a corresponding '{'";
    }
    
    return suggestions;
}

void ParserBase::skipToRecoveryPoint() {
    // Skip to next semicolon, brace, or end of input for error recovery
    while (hasTokens() && 
           peekToken().type != TokenType::Semicolon && 
           peekToken().type != TokenType::LeftBrace && 
           peekToken().type != TokenType::RightBrace) {
        advance();
    }
    
    if (hasTokens() && peekToken().type == TokenType::Semicolon) {
        advance(); // Skip the semicolon
    }
}

int ParserBase::parseInteger(const std::string& str) const {
    int value;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
    if (ec != std::errc{}) {
        reportError("Invalid integer format: '" + str + "'. Expected a valid number like 42 or -123.");
    }
    return value;
}

[[noreturn]] void ParserBase::reportError(const std::string& message) const {
    std::cerr << "\nParse Error " << getErrorContext() << ":\n";
    std::cerr << "   " << message << "\n";
    
    // Add context-aware suggestions
    std::string suggestions = getSuggestions();
    if (!suggestions.empty()) {
        std::cerr << suggestions << "\n";
    }
    
    // Show surrounding context if available
    if (current > 0 && current < size) {
        std::cerr << "\nContext:\n";
        
        // Show previous token
        if (current > 0) {
            const auto& prevToken = tokens[current - 1];
            std::cerr << "   Previous: " << tokenTypeToString(prevToken.type);
            if (prevToken.value.has_value()) {
                std::cerr << " '" << prevToken.value.value() << "'";
            }
            std::cerr << "\n";
        }
        
        // Show current token
        const auto& currToken = tokens[current];
        std::cerr << "   Current:  " << tokenTypeToString(currToken.type);
        if (currToken.value.has_value()) {
            std::cerr << " '" << currToken.value.value() << "'";
        }
        std::cerr << " ← ERROR HERE\n";
        
        // Show next token if available
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

void ParserBase::reportErrorWithRecovery(const std::string& message) {
    std::cerr << "\nParse Warning " << getErrorContext() << ":\n";
    std::cerr << "   " << message << "\n";
    
    // Add suggestions
    std::string suggestions = getSuggestions();
    if (!suggestions.empty()) {
        std::cerr << suggestions << "\n";
    }
    
    std::cerr << "   Attempting to recover...\n" << std::endl;
    skipToRecoveryPoint();
}

[[noreturn]] void ParserBase::reportExpectedError(TokenType expected, const std::string& context) const {
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
    
    // Add specific help for common mistakes
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
