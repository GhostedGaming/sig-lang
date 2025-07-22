#include <parser/public/parser.hpp>
#include <iostream>
#include <cstdlib>
#include <string_view>
#include <charconv>

// Parser class to encapsulate state and avoid passing parameters repeatedly
class Parser {
private:
    const std::vector<Token>& tokens;
    size_t current;
    const size_t size;

    // Fast integer parsing using std::from_chars (C++17)
    int parseInteger(std::string_view str) const {
        int value;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
        if (ec != std::errc{}) {
            reportError("Invalid integer format");
        }
        return value;
    }

    // Centralized error reporting with better context
    [[noreturn]] void reportError(const char* message) const {
        std::cerr << "Parser error at token " << current << ": " << message << '\n';
        std::exit(1);
    }

    // Inline bounds checking to avoid repeated checks
    inline bool hasTokens(size_t count = 1) const {
        return current + count <= size;
    }

    // Safe token access with bounds checking
    inline const Token& peekToken(size_t offset = 0) const {
        if (current + offset >= size) {
            reportError("Unexpected end of input");
        }
        return tokens[current + offset];
    }

    // Advance current position
    inline void advance(size_t count = 1) {
        current += count;
    }

    // Expect specific token type and advance
    inline void expectToken(TokenType expected, const char* error_msg) {
        if (!hasTokens() || peekToken().type != expected) {
            reportError(error_msg);
        }
        advance();
    }

public:
    explicit Parser(const std::vector<Token>& tokens) 
        : tokens(tokens), current(0), size(tokens.size()) {
    }

    void parseReturnStatement(AST& ast) {
        advance();
        
        // Expect integer literal
        if (!hasTokens() || peekToken().type != TokenType::IntegerLiteral) {
            reportError("Expected integer literal after 'return'");
        }
        
        const auto& token = peekToken();
        int value;
        
        // Use optional to avoid exception handling overhead
        if (token.value.has_value()) {
            value = parseInteger(token.value.value());
        } else {
            reportError("Integer literal missing value");
        }
        
        advance(); // Skip integer
        
        // Expect semicolon
        expectToken(TokenType::Semicolon, "Expected semicolon after integer");
        
        // Use emplace_back for better performance
        ast.emplace_back(ReturnStatement{value});
    }

    void parsePrintStatement(AST& ast) {
        advance(); // Skip 'print'
        
        // Expect left parenthesis
        expectToken(TokenType::LeftParen, "Expected '(' after 'print'");
        
        // Expect opening quote
        expectToken(TokenType::Quote, "Expected '\"' after 'print('");
        
        // Expect either integer literal OR string (use && for proper logic)
        if (!hasTokens() || (peekToken().type != TokenType::IntegerLiteral && peekToken().type != TokenType::String)) {
            reportError("Expected integer literal or string in print statement");
        }

        const auto& token = peekToken();

        // Handle both integers and strings
        if (token.type == TokenType::IntegerLiteral) {
            if (token.value.has_value()) {
                int value = parseInteger(token.value.value());
                advance();
                expectToken(TokenType::Quote, "Expected '\"' after integer");
                expectToken(TokenType::RightParen, "Expected ')' after closing quote");
                expectToken(TokenType::Semicolon, "Expected semicolon after print statement");

                ast.emplace_back(PrintStatement{value});
            } else {
                reportError("Integer literal missing value");
            }
        } else if (token.type == TokenType::String) {
            if (token.value.has_value()) {
                std::string stringValue = token.value.value();
                advance();
                expectToken(TokenType::Quote, "Expected '\"' after string");
                expectToken(TokenType::RightParen, "Expected ')' after closing quote");
                expectToken(TokenType::Semicolon, "Expected semicolon after print statement");

                ast.emplace_back(PrintStatement{stringValue});
            } else {
                reportError("String literal missing value");
            }
        }
    }

    // The most unsafe function i will make
    void parseAsmStatement(AST& ast) {
        advance(); // Skip 'asm'

        expectToken(TokenType::LeftParen, "Expected '(' after 'asm'");
        
        // Expect opening quote
        expectToken(TokenType::Quote, "Expected '\"' after asm(\"");

        if (!hasTokens() || (peekToken().type != TokenType::String)) {
            reportError("Expected string in asm statement");
        }

        const auto& token = peekToken();

        if (token.type == TokenType::String) {
            if (token.value.has_value()) {
                std::string stringValue = token.value.value();
                advance();
                expectToken(TokenType::Quote, "Expected '\"' after string");
                expectToken(TokenType::RightParen, "Expected ')' after closing quote");
                expectToken(TokenType::Semicolon, "Expected semicolon after asm statement");

                ast.emplace_back(AsmStatement{stringValue});
            } else {
                reportError("String literal missing value");
            }
        }
    }

    void parseStatement(AST& ast) {
        if (!hasTokens()) {
            reportError("Unexpected end of input");
        }
        
        switch (peekToken().type) {
            case TokenType::KeywordReturn:
                parseReturnStatement(ast);
                break;
            case TokenType::KeywordPrint:
                parsePrintStatement(ast);
                break;
            case TokenType::KeywordAsm:
                parseAsmStatement(ast);
                break;
            case TokenType::Comment:
                break; // Ignore comments
            default:
                reportError("Unexpected token type");
        }
    }

    AST parse() {
        AST ast;
        
        // Estimate AST size to reduce allocations
        ast.reserve(tokens.size() / 3); // Rough estimate: each statement ~3 tokens
        
        while (hasTokens()) {
            parseStatement(ast);
        }
        
        return ast;
    }
};

// Optimized parse function - now just creates parser and delegates
AST parse(const std::vector<Token>& tokens) {
    if (tokens.empty()) {
        return AST{}; // Early return for empty input
    }
    
    Parser parser(tokens);
    return parser.parse();
}