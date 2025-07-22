#include <parser/public/parser.hpp>
#include <iostream>
#include <cstdlib>
#include <string_view>
#include <charconv>

/**
 * Parser class to encapsulate parsing state and provide structured parsing
 * Uses recursive descent parsing with lookahead for syntax analysis
 */
class Parser {
private:
    const std::vector<Token>& tokens; // Reference to input token stream
    size_t current; // Current position in token stream
    const size_t size; // Total number of tokens

    /**
     * Fast integer parsing using std::from_chars (C++17)
     * Avoids string-to-int conversion overhead of std::stoi
     */
    int parseInteger(std::string_view str) const {
        int value;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
        if (ec != std::errc{}) {
            reportError("Invalid integer format");
        }
        return value;
    }

    /**
     * Centralized error reporting with parser context
     * Provides consistent error messages and terminates parsing
     */
    [[noreturn]] void reportError(const char* message) const {
        std::cerr << "Parser error at token " << current << ": " << message << '\n';
        std::exit(1);
    }

    /**
     * Inline bounds checking to prevent buffer overruns
     * Checks if we have enough tokens remaining for parsing
     */
    inline bool hasTokens(size_t count = 1) const {
        return current + count <= size;
    }

    /**
     * Safe token access with bounds checking
     * Allows lookahead without risking out-of-bounds access
     */
    inline const Token& peekToken(size_t offset = 0) const {
        if (current + offset >= size) {
            reportError("Unexpected end of input");
        }
        return tokens[current + offset];
    }

    /**
     * Advance parser position in token stream
     * Moves current pointer forward by specified count
     */
    inline void advance(size_t count = 1) {
        current += count;
    }

    /**
     * Expect specific token type and advance
     * Validates token type and consumes it, or reports error
     */
    inline void expectToken(TokenType expected, const char* error_msg) {
        if (!hasTokens() || peekToken().type != expected) {
            reportError(error_msg);
        }
        advance();
    }

public:
    /**
     * Constructor - Initialize parser with token stream
     */
    explicit Parser(const std::vector<Token>& tokens)
        : tokens(tokens), current(0), size(tokens.size()) {}

    /**
     * Parse return statement: return <integer>;
     * Grammar: 'return' IntegerLiteral ';'
     */
    void parseReturnStatement(AST& ast) {
        advance(); // Skip 'return' keyword

        // Expect integer literal
        if (!hasTokens() || peekToken().type != TokenType::IntegerLiteral) {
            reportError("Expected integer literal after 'return'");
        }

        const auto& token = peekToken();
        int value;

        // Parse integer value with error checking
        if (token.value.has_value()) {
            value = parseInteger(token.value.value());
        } else {
            reportError("Integer literal missing value");
        }

        advance(); // Skip integer literal

        // Expect semicolon terminator
        expectToken(TokenType::Semicolon, "Expected semicolon after integer");

        // Add return statement to AST
        ast.emplace_back(ReturnStatement{value});
    }

    /**
     * Parse print statement: print("string") or print(42) or print(variable);
     * Grammar: 'print' '(' (String | IntegerLiteral | Identifier) ')' ';'
     */
    void parsePrintStatement(AST& ast) {
        advance(); // Skip 'print' keyword

        // Expect opening parenthesis
        expectToken(TokenType::LeftParen, "Expected '(' after 'print'");

        // Check what type of content we have
        if (!hasTokens()) {
            reportError("Expected content in print statement");
        }

        const auto& token = peekToken();

        // Handle quoted strings: print("hello")
        if (token.type == TokenType::Quote) {
            advance(); // Skip opening quote

            // Expect string content
            if (!hasTokens() || peekToken().type != TokenType::String) {
                reportError("Expected string content after opening quote");
            }

            const auto& stringToken = peekToken();
            if (stringToken.value.has_value()) {
                std::string stringValue = stringToken.value.value();
                advance(); // Skip string content
                expectToken(TokenType::Quote, "Expected closing '\"' after string");
                expectToken(TokenType::RightParen, "Expected ')' after closing quote");
                expectToken(TokenType::Semicolon, "Expected semicolon after print statement");
                ast.emplace_back(PrintStatement{stringValue});
            } else {
                reportError("String literal missing value");
            }
        }
        // Handle direct integers: print(42)
        else if (token.type == TokenType::IntegerLiteral) {
            if (token.value.has_value()) {
                int value = parseInteger(token.value.value());
                advance(); // Skip integer
                expectToken(TokenType::RightParen, "Expected ')' after integer");
                expectToken(TokenType::Semicolon, "Expected semicolon after print statement");
                ast.emplace_back(PrintStatement{value});
            } else {
                reportError("Integer literal missing value");
            }
        }
        // Handle variables: print(variableName)
        else if (token.type == TokenType::Identifier) {
            if (token.value.has_value()) {
                std::string variableName = token.value.value();
                advance(); // Skip variable name
                expectToken(TokenType::RightParen, "Expected ')' after variable");
                expectToken(TokenType::Semicolon, "Expected semicolon after print statement");
                ast.emplace_back(PrintVariable{variableName});
            } else {
                reportError("Variable name missing value");
            }
        }
        else {
            reportError("Expected string, integer, or variable in print statement");
        }
    }

    /**
     * Parse function definition: [pub] fn name() { ... }
     * Grammar: ['pub'] 'fn' Identifier '(' ')' '{' StatementList '}'
     */
    void parseFunctionDefinition(AST& ast) {
        // Handle optional 'pub' modifier
        bool isPublic = false;
        if (hasTokens() && peekToken().type == TokenType::KeywordPub) {
            isPublic = true;
            advance(); // Skip 'pub'
        }

        // Expect 'fn' keyword
        expectToken(TokenType::Function, "Expected 'fn' keyword");

        // Expect function name (identifier)
        if (!hasTokens() || peekToken().type != TokenType::Identifier) {
            reportError("Expected function name after 'fn'");
        }

        const std::string functionName = peekToken().value.value_or("unnamed");
        advance(); // Skip function name

        // Expect parameter list (currently empty)
        expectToken(TokenType::LeftParen, "Expected '(' after function name");
        expectToken(TokenType::RightParen, "Expected ')' after function parameters");

        // Expect function body
        expectToken(TokenType::LeftBrace, "Expected '{' after function signature");

        // Parse function body statements
        AST functionBody;
        parseStatementList(functionBody);

        // Expect closing brace
        expectToken(TokenType::RightBrace, "Expected '}' after function body");

        // Add function definition to AST
        ast.emplace_back(FunctionDefinition{functionName, std::move(functionBody)});
    }

    /**
     * Parse function call: functionName();
     * Grammar: Identifier '(' ')' ';'
     */
    void parseFunctionCall(AST& ast) {
        // First capture the function name before advancing
        if (!hasTokens() || peekToken().type != TokenType::Identifier) {
            reportError("Expected function name");
        }

        const std::string functionName = peekToken().value.value_or("unnamed");
        advance(); // Now skip the function name

        expectToken(TokenType::LeftParen, "Expected '(' after function name");
        expectToken(TokenType::RightParen, "Expected ')' after function arguments");
        expectToken(TokenType::Semicolon, "Expected ';' after function call");

        ast.emplace_back(FunctionCall{functionName});
    }

    /**
     * Parse inline assembly statement: asm("assembly code");
     * Grammar: 'asm' '(' '"' String '"' ')' ';'
     * Note: This allows direct assembly injection - use with caution
     */
    void parseAsmStatement(AST& ast) {
        advance(); // Skip 'asm' keyword

        expectToken(TokenType::LeftParen, "Expected '(' after 'asm'");
        expectToken(TokenType::Quote, "Expected '\"' after 'asm('");

        // Expect assembly string content
        if (!hasTokens() || peekToken().type != TokenType::String) {
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

    /**
     * Parse variable declaration/assignment: let name; or let name = value;
     * Grammar: 'let' Identifier ['=' (IntegerLiteral | String)] ';'
     * Fixed to handle both quoted and unquoted strings properly
     */
    void parseVariables(AST& ast) {
        advance(); // Skip 'let' keyword

        // Expect variable name (identifier)
        if (!hasTokens() || peekToken().type != TokenType::Identifier) {
            reportError("Expected variable name after keyword 'let'");
        }

        const std::string variableName = peekToken().value.value_or("unnamed");
        advance(); // Skip variable name

        // Check if there's an assignment (=)
        if (hasTokens() && peekToken().type == TokenType::Equal) {
            advance(); // Skip '=' token

            // Expect a value after '='
            if (!hasTokens()) {
                reportError("Expected value after '='");
            }

            const auto& valueToken = peekToken();

            // Handle different value types
            if (valueToken.type == TokenType::IntegerLiteral) {
                if (valueToken.value.has_value()) {
                    int value = parseInteger(valueToken.value.value());
                    advance(); // Skip the value
                    expectToken(TokenType::Semicolon, "Expected semicolon after variable assignment");

                    // Add variable assignment to AST
                    ast.emplace_back(VariableAssignment{variableName, value});
                } else {
                    reportError("Integer literal missing value");
                }
            }
            // Handle quoted strings: let x = "hello";
            else if (valueToken.type == TokenType::Quote) {
                advance(); // Skip opening quote

                if (!hasTokens() || peekToken().type != TokenType::String) {
                    reportError("Expected string content after opening quote");
                }

                const auto& stringToken = peekToken();
                if (stringToken.value.has_value()) {
                    std::string stringValue = stringToken.value.value();
                    advance(); // Skip string content
                    expectToken(TokenType::Quote, "Expected closing '\"' after string");
                    expectToken(TokenType::Semicolon, "Expected semicolon after variable assignment");

                    // Add variable assignment to AST
                    ast.emplace_back(VariableAssignment{variableName, stringValue});
                } else {
                    reportError("String literal missing value");
                }
            }
            // Handle direct string tokens (if lexer produces them differently)
            else if (valueToken.type == TokenType::String) {
                if (valueToken.value.has_value()) {
                    std::string stringValue = valueToken.value.value();
                    advance(); // Skip the value
                    expectToken(TokenType::Semicolon, "Expected semicolon after variable assignment");

                    // Add variable assignment to AST
                    ast.emplace_back(VariableAssignment{variableName, stringValue});
                } else {
                    reportError("String literal missing value");
                }
            }
            else {
                reportError("Expected integer or string value after '='");
            }
        } else {
            // No assignment, just variable declaration
            expectToken(TokenType::Semicolon, "Expected semicolon after variable declaration");

            // Add variable declaration to AST (without value)
            ast.emplace_back(VariableDeclaration{variableName});
        }
    }

    /**
     * Parse a list of statements until closing brace or end of input
     * Used for parsing function bodies and block statements
     */
    void parseStatementList(AST& ast) {
        while (hasTokens() && peekToken().type != TokenType::RightBrace) {
            parseStatement(ast);
        }
    }

    /**
     * Parse individual statement based on leading token
     * Dispatches to appropriate statement parser
     */
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

            case TokenType::KeywordPub:
            case TokenType::Function:
                parseFunctionDefinition(ast);
                break;

            case TokenType::Comment:
                advance(); // Skip comments
                break;

            case TokenType::Identifier:
                // Look ahead to determine if it's a function call
                if (hasTokens(2) && peekToken(1).type == TokenType::LeftParen) {
                    parseFunctionCall(ast);
                } else {
                    reportError("Unexpected identifier - did you mean to call a function or declare a variable with 'let'?");
                }
                break;

            case TokenType::KeywordLet:
                parseVariables(ast);
                break;

            default:
                reportError("Unexpected token type");
        }
    }

    /**
     * Main parsing entry point
     * Parses entire token stream into AST
     */
    AST parse() {
        AST ast;
        // Reserve space to reduce allocations (rough estimate)
        ast.reserve(tokens.size() / 3);

        // Parse all statements until end of input
        while (hasTokens()) {
            parseStatement(ast);
        }

        return ast;
    }
};

/**
/**
 * Public parsing interface
 * Creates parser instance and delegates to internal parser
 */
AST parse(const std::vector<Token>& tokens) {
    if (tokens.empty()) {
        return AST{}; // Return empty AST for empty input
    }

    Parser parser(tokens);
    return parser.parse();
}
