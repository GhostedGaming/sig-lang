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
            reportError("Invalid integer format: '" + std::string(str) + "'. Expected a valid number like 42 or -123.");
        }
        return value;
    }

    /**
     * Get a string representation of a token type for error messages
     */
    std::string tokenTypeToString(TokenType type) const {
        switch (type) {
            case TokenType::KeywordReturn: return "'return'";
            case TokenType::KeywordPrint: return "'print'";
            case TokenType::KeywordPub: return "'pub'";
            case TokenType::KeywordAsm: return "'asm'";
            case TokenType::KeywordLet: return "'let'";
            case TokenType::Function: return "'fn'";
            case TokenType::Identifier: return "identifier";
            case TokenType::IntegerLiteral: return "integer";
            case TokenType::String: return "string";
            case TokenType::Quote: return "'\"'";
            case TokenType::LeftParen: return "'('";
            case TokenType::RightParen: return "')'";
            case TokenType::LeftBrace: return "'{'";
            case TokenType::RightBrace: return "'}'";
            case TokenType::Semicolon: return "';'";
            case TokenType::Equal: return "'='";
            case TokenType::Comment: return "comment";
            case TokenType::MultilineComment: return "multiline comment";
            case TokenType::EndMultilineComment: return "'*/'";
            default: return "unknown token";
        }
    }

    /**
     * Get context information for error messages
     */
    std::string getErrorContext() const {
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

    /**
     * Enhanced error recovery - skip tokens until we find a recovery point
     */
    void skipToRecoveryPoint() {
        while (hasTokens()) {
            const auto& token = peekToken();
            // Recovery points: semicolons, braces, or statement keywords
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
    
    /**
     * Get helpful suggestions based on context
     */
    std::string getSuggestions() const {
        if (!hasTokens()) return "";
        
        const auto& token = peekToken();
        std::string suggestions;
        
        // Context-aware suggestions
        if (current > 0) {
            const auto& prevToken = tokens[current - 1];
            
            // After let keyword
            if (prevToken.type == TokenType::KeywordLet) {
                suggestions = "\nSuggestions:\n   • let variableName;\n   • let x = 42;\n   • let name = \"value\";";
            }
            // After print keyword  
            else if (prevToken.type == TokenType::KeywordPrint) {
                suggestions = "\nSuggestions:\n   • print(\"Hello\");\n   • print(42);\n   • print(variableName);";
            }
            // After if keyword
            else if (prevToken.type == TokenType::KeywordIf) {
                suggestions = "\nSuggestions:\n   • if (x == 5) { ... }\n   • if (name != \"test\") { ... }";
            }
            // After identifier
            else if (prevToken.type == TokenType::Identifier && token.type != TokenType::LeftParen) {
                suggestions = "\nDid you mean:\n   • " + prevToken.value.value_or("name") + "(); (function call)\n   • let " + prevToken.value.value_or("name") + " = value; (assignment)";
            }
        }
        
        return suggestions;
    }
    
    /**
     * Centralized error reporting with enhanced context and suggestions
     * Provides detailed error messages with location and helpful suggestions
     */
    [[noreturn]] void reportError(const std::string& message) const {
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
    
    /**
     * Non-fatal error reporting with recovery
     */
    void reportErrorWithRecovery(const std::string& message) {
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

    /**
     * Report error with expected vs actual token information
     */
    [[noreturn]] void reportExpectedError(TokenType expected, const std::string& context = "") const {
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
            reportError("Unexpected end of input. Expected more tokens to complete the statement.");
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
    inline void expectToken(TokenType expected, const std::string& context = "") {
        if (!hasTokens() || peekToken().type != expected) {
            reportExpectedError(expected, context);
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
        int value;

        // Expect integer literal
        if (!hasTokens() || peekToken().type != TokenType::IntegerLiteral) {
            std::cerr << "⚠️  Warning: Expected integer literal after 'return'. ";
            std::cerr << "Examples: 'return 0;' or 'return 42;'\n";
            std::cerr << "   Defaulting return value to 0\n\n";
            value = 0;
        } else {
            const auto& token = peekToken();
            if (token.value.has_value()) {
                value = parseInteger(token.value.value());
            } else {
                reportError("Integer literal is missing its value. This appears to be a tokenizer issue.");
            }
            advance(); // Skip integer literal
        }

        // Expect semicolon terminator (but handle gracefully if missing)
        if (hasTokens() && peekToken().type == TokenType::Semicolon) {
            advance(); // Skip semicolon
        } else {
            std::cerr << "⚠️  Warning: Missing semicolon ';' after return statement. ";
            std::cerr << "All statements should end with a semicolon.\n\n";
        }

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
        expectToken(TokenType::LeftParen, "after 'print'. Syntax: print(\"hello\") or print(42)");

        // Check what type of content we have
        if (!hasTokens()) {
            reportError("Expected content inside print statement parentheses.\n"
                       "   Examples:\n"
                       "   • print(\"Hello World\");\n"
                       "   • print(42);\n"
                       "   • print(myVariable);");
        }

        const auto& token = peekToken();

        // Handle quoted strings: print("hello")
        if (token.type == TokenType::Quote) {
            advance(); // Skip opening quote

            // Expect string content
            if (!hasTokens() || peekToken().type != TokenType::String) {
                reportError("Expected string content after opening quote.\n"
                           "   Example: print(\"Hello World\");");
            }

            const auto& stringToken = peekToken();
            if (stringToken.value.has_value()) {
                std::string stringValue = stringToken.value.value();
                advance(); // Skip string content
                expectToken(TokenType::Quote, "after string content to close the string");
                expectToken(TokenType::RightParen, "after closing quote to end print statement");
                expectToken(TokenType::Semicolon, "to end print statement");
                ast.emplace_back(PrintStatement{stringValue});
            } else {
                reportError("String literal is missing its value. This appears to be a tokenizer issue.");
            }
        }
        // Handle direct integers: print(42)
        else if (token.type == TokenType::IntegerLiteral) {
            if (token.value.has_value()) {
                int value = parseInteger(token.value.value());
                advance(); // Skip integer
                expectToken(TokenType::RightParen, "after integer to end print statement");
                expectToken(TokenType::Semicolon, "to end print statement");
                ast.emplace_back(PrintStatement{value});
            } else {
                reportError("Integer literal is missing its value. This appears to be a tokenizer issue.");
            }
        }
        // Handle variables: print(variableName)
        else if (token.type == TokenType::Identifier) {
            if (token.value.has_value()) {
                std::string variableName = token.value.value();
                advance(); // Skip variable name
                expectToken(TokenType::RightParen, "after variable name to end print statement");
                expectToken(TokenType::Semicolon, "to end print statement");
                ast.emplace_back(PrintVariable{variableName});
            } else {
                reportError("Variable name is missing its value. This appears to be a tokenizer issue.");
            }
        }
        else {
            reportError("Expected string, integer, or variable name inside print statement.\n"
                       "   Valid examples:\n"
                       "   • print(\"Hello\");     (string)\n"
                       "   • print(42);           (integer)\n"
                       "   • print(myVar);        (variable)\n"
                       "   Found: " + tokenTypeToString(token.type));
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
        expectToken(TokenType::Function, "when defining a function. Use: fn functionName() { ... }");

        // Expect function name (identifier)
        if (!hasTokens() || peekToken().type != TokenType::Identifier) {
            reportError("Expected function name after 'fn' keyword.\n"
                       "   Example: fn myFunction() { return 0; }");
        }

        const std::string functionName = peekToken().value.value_or("unnamed");
        advance(); // Skip function name

        // Expect parameter list (currently empty)
        expectToken(TokenType::LeftParen, "after function name. Currently only empty parameter lists '()' are supported");
        expectToken(TokenType::RightParen, "to close function parameter list");

        // Expect function body
        expectToken(TokenType::LeftBrace, "to start function body. Example: fn test() { return 0; }");

        // Parse function body statements
        AST functionBody;
        parseStatementList(functionBody);

        // Expect closing brace
        expectToken(TokenType::RightBrace, "to close function body");

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
            reportError("Expected function name for function call.\n"
                       "   Example: myFunction();");
        }

        const std::string functionName = peekToken().value.value_or("unnamed");
        advance(); // Now skip the function name

        expectToken(TokenType::LeftParen, "after function name. Example: " + functionName + "();");
        expectToken(TokenType::RightParen, "to close function call. Currently only parameterless calls are supported");
        expectToken(TokenType::Semicolon, "to end function call statement");

        ast.emplace_back(FunctionCall{functionName});
    }

    /**
     * Parse inline assembly statement: asm("assembly code");
     * Grammar: 'asm' '(' '"' String '"' ')' ';'
     * Note: This allows direct assembly injection - use with caution
     */
    void parseAsmStatement(AST& ast) {
        advance(); // Skip 'asm' keyword

        expectToken(TokenType::LeftParen, "after 'asm'. Syntax: asm(\"mov eax, 1\");");
        expectToken(TokenType::Quote, "to start assembly string. Example: asm(\"nop\");");

        // Expect assembly string content
        if (!hasTokens() || peekToken().type != TokenType::String) {
            reportError("Expected assembly instruction string after opening quote.\n"
                       "   Example: asm(\"mov eax, 1\");");
        }

        const auto& token = peekToken();
        if (token.type == TokenType::String) {
            if (token.value.has_value()) {
                std::string stringValue = token.value.value();
                advance();
                expectToken(TokenType::Quote, "to close assembly string");
                expectToken(TokenType::RightParen, "to close asm statement");
                expectToken(TokenType::Semicolon, "to end asm statement");
                ast.emplace_back(AsmStatement{stringValue});
            } else {
                reportError("Assembly string is missing its value. This appears to be a tokenizer issue.");
            }
        }
    }

    void parseIfStatement(AST& ast) {
        advance(); // Skip 'if'

        expectToken(TokenType::LeftParen, "after 'if' keyword. Syntax: if (condition) { ... }");

        // Parse condition: left operand
        if (!hasTokens()) {
            reportError("Expected condition after '(' in if statement");
            return;
        }
        
        IfStatement ifStmt;
        Token leftToken = peekToken();
        
        if (leftToken.type == TokenType::Identifier || leftToken.type == TokenType::IntegerLiteral || leftToken.type == TokenType::String) {
            ifStmt.left = leftToken.value.value_or("");
            advance();
        } else {
            reportError("Expected identifier, number, or string as left operand in if condition");
            return;
        }
        
        // Parse comparison operator
        if (!hasTokens()) {
            reportError("Expected comparison operator in if condition");
            return;
        }
        
        Token opToken = peekToken();
        switch (opToken.type) {
            case TokenType::EqualEqual:
                ifStmt.op = "==";
                break;
            case TokenType::NotEqual:
                ifStmt.op = "!=";
                break;
            case TokenType::LessThan:
                ifStmt.op = "<";
                break;
            case TokenType::LessThanEqual:
                ifStmt.op = "<=";
                break;
            case TokenType::GreaterThan:
                ifStmt.op = ">";
                break;
            case TokenType::GreaterThanEqual:
                ifStmt.op = ">=";
                break;
            default:
                reportError("Expected comparison operator (==, !=, <, <=, >, >=) in if condition");
                return;
        }
        advance();
        
        // Parse right operand
        if (!hasTokens()) {
            reportError("Expected right operand after comparison operator in if condition");
            return;
        }
        
        Token rightToken = peekToken();
        if (rightToken.type == TokenType::Identifier || rightToken.type == TokenType::IntegerLiteral || rightToken.type == TokenType::String) {
            ifStmt.right = rightToken.value.value_or("");
            advance();
        } else {
            reportError("Expected identifier, number, or string as right operand in if condition");
            return;
        }
        
        expectToken(TokenType::RightParen, "after if condition. Expected closing ')'");
        
        // Parse then block
        expectToken(TokenType::LeftBrace, "after if condition. Expected opening '{'");
        
        // Parse statements inside the then block
        parseStatementList(ifStmt.thenBlock);
        
        expectToken(TokenType::RightBrace, "after if then block. Expected closing '}'");

        // Parse elif clauses
        while (hasTokens() && peekToken().type == TokenType::KeywordElif) {
            advance(); // Skip 'elif'
            
            ElifClause elifClause;
            
            expectToken(TokenType::LeftParen, "after 'elif' keyword. Expected opening '('");
            
            // Parse elif condition (same as if condition)
            if (!hasTokens()) {
                reportError("Expected condition after '(' in elif statement");
                return;
            }
            
            Token leftToken = peekToken();
            if (leftToken.type == TokenType::Identifier || leftToken.type == TokenType::IntegerLiteral || leftToken.type == TokenType::String) {
                elifClause.left = leftToken.value.value_or("");
                advance();
            } else {
                reportError("Expected identifier, number, or string as left operand in elif condition");
                return;
            }
            
            // Parse comparison operator
            if (!hasTokens()) {
                reportError("Expected comparison operator in elif condition");
                return;
            }
            
            Token opToken = peekToken();
            switch (opToken.type) {
                case TokenType::EqualEqual:
                    elifClause.op = "==";
                    break;
                case TokenType::NotEqual:
                    elifClause.op = "!=";
                    break;
                case TokenType::LessThan:
                    elifClause.op = "<";
                    break;
                case TokenType::LessThanEqual:
                    elifClause.op = "<=";
                    break;
                case TokenType::GreaterThan:
                    elifClause.op = ">";
                    break;
                case TokenType::GreaterThanEqual:
                    elifClause.op = ">=";
                    break;
                default:
                    reportError("Expected comparison operator (==, !=, <, <=, >, >=) in elif condition");
                    return;
            }
            advance();
            
            // Parse right operand
            if (!hasTokens()) {
                reportError("Expected right operand after comparison operator in elif condition");
                return;
            }
            
            Token rightToken = peekToken();
            if (rightToken.type == TokenType::Identifier || rightToken.type == TokenType::IntegerLiteral || rightToken.type == TokenType::String) {
                elifClause.right = rightToken.value.value_or("");
                advance();
            } else {
                reportError("Expected identifier, number, or string as right operand in elif condition");
                return;
            }
            
            expectToken(TokenType::RightParen, "after elif condition. Expected closing ')'");
            expectToken(TokenType::LeftBrace, "after elif condition. Expected opening '{'");
            
            // Parse elif block
            parseStatementList(elifClause.block);
            
            expectToken(TokenType::RightBrace, "after elif block. Expected closing '}'");
            
            ifStmt.elifClauses.push_back(elifClause);
        }
        
        // Check for optional else block
        if (hasTokens() && peekToken().type == TokenType::KeywordElse) {
            advance(); // Skip 'else'
            expectToken(TokenType::LeftBrace, "after else keyword. Expected opening '{'");
            
            // Initialize else block
            ifStmt.elseBlock = std::vector<ASTNode>();
            
            // Parse statements inside the else block
            parseStatementList(ifStmt.elseBlock.value());
            
            expectToken(TokenType::RightBrace, "after else block. Expected closing '}'");
        }
        
        ast.push_back(ifStmt);
    }

    void parseWhile(AST& ast) {
        advance(); // Skip 'while'
        expectToken(TokenType::LeftParen, "after while keyword. Expected opening '('");

        // Parse the condition - could be single operand or binary comparison
        if (!hasTokens()) {
            reportError("Expected condition after '(' in while statement");
            return;
        }
        
        WhileStatement whileStmt;
        
        // Parse first operand (left side or single condition)
        Token firstToken = peekToken();
        if (firstToken.type != TokenType::Identifier && 
            firstToken.type != TokenType::IntegerLiteral && 
            firstToken.type != TokenType::String) {
            reportError("Expected identifier, number, or string in while condition");
            return;
        }
        
        whileStmt.left = firstToken.value.value_or("");
        advance();
        
        // Check if there's a comparison operator next
        if (hasTokens() && peekToken().type != TokenType::RightParen) {
            Token opToken = peekToken();
            
            // Check if it's a comparison operator
            bool isComparisonOp = (opToken.type == TokenType::EqualEqual ||
                                 opToken.type == TokenType::NotEqual ||
                                 opToken.type == TokenType::LessThan ||
                                 opToken.type == TokenType::LessThanEqual ||
                                 opToken.type == TokenType::GreaterThan ||
                                 opToken.type == TokenType::GreaterThanEqual);
            
            if (isComparisonOp) {
                // Binary comparison: while (left op right)
                switch (opToken.type) {
                    case TokenType::EqualEqual: whileStmt.op = "=="; break;
                    case TokenType::NotEqual: whileStmt.op = "!="; break;
                    case TokenType::LessThan: whileStmt.op = "<"; break;
                    case TokenType::LessThanEqual: whileStmt.op = "<="; break;
                    case TokenType::GreaterThan: whileStmt.op = ">"; break;
                    case TokenType::GreaterThanEqual: whileStmt.op = ">="; break;
                    default: break;
                }
                advance(); // consume operator
                
                // Parse right operand
                if (!hasTokens()) {
                    reportError("Expected right operand after comparison operator in while condition");
                    return;
                }
                
                Token rightToken = peekToken();
                if (rightToken.type != TokenType::Identifier && 
                    rightToken.type != TokenType::IntegerLiteral && 
                    rightToken.type != TokenType::String) {
                    reportError("Expected identifier, number, or string as right operand in while condition");
                    return;
                }
                
                whileStmt.right = rightToken.value.value_or("");
                advance();
            } else {
                reportError("Unexpected token in while condition. Expected comparison operator or closing ')'");
                return;
            }
        } else {
            // Single operand condition (e.g., while (1) or while (x))
            whileStmt.op = "";
            whileStmt.right = "";
        }
        
        expectToken(TokenType::RightParen, "after while condition. Expected closing ')'");
        expectToken(TokenType::LeftBrace, "after while condition. Expected opening '{'");
        
        // Parse while body block
        parseStatementList(whileStmt.body);
        
        expectToken(TokenType::RightBrace, "after while body. Expected closing '}'");
        
        // Add whileStmt to AST
        ast.push_back(whileStmt);
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
            reportError("Expected variable name after 'let' keyword.\n"
                       "   Examples:\n"
                       "   • let myVar;\n"
                       "   • let count = 42;\n"
                       "   • let name = \"John\";");
        }

        const std::string variableName = peekToken().value.value_or("unnamed");
        advance(); // Skip variable name

        // Check if there's an assignment (=)
        if (hasTokens() && peekToken().type == TokenType::Equal) {
            advance(); // Skip '=' token

            // Expect a value after '='
            if (!hasTokens()) {
                reportError("Expected value after '=' in variable assignment.\n"
                           "   Examples:\n"
                           "   • let x = 42;\n"
                           "   • let text = \"hello\";");
            }

            const auto& valueToken = peekToken();

            // Handle different value types
            if (valueToken.type == TokenType::IntegerLiteral) {
                if (valueToken.value.has_value()) {
                    int value = parseInteger(valueToken.value.value());
                    advance(); // Skip the value
                    expectToken(TokenType::Semicolon, "to end variable assignment");

                    // Add variable assignment to AST
                    ast.emplace_back(VariableAssignment{variableName, value});
                } else {
                    reportError("Integer literal is missing its value. This appears to be a tokenizer issue.");
                }
            }
            // Handle quoted strings: let x = "hello";
            else if (valueToken.type == TokenType::Quote) {
                advance(); // Skip opening quote

                if (!hasTokens() || peekToken().type != TokenType::String) {
                    reportError("Expected string content after opening quote in variable assignment.\n"
                               "   Example: let name = \"John\";");
                }

                const auto& stringToken = peekToken();
                if (stringToken.value.has_value()) {
                    std::string stringValue = stringToken.value.value();
                    advance(); // Skip string content
                    expectToken(TokenType::Quote, "to close string in variable assignment");
                    expectToken(TokenType::Semicolon, "to end variable assignment");

                    // Add variable assignment to AST
                    ast.emplace_back(VariableAssignment{variableName, stringValue});
                } else {
                    reportError("String literal is missing its value. This appears to be a tokenizer issue.");
                }
            }
            // Handle direct string tokens (if lexer produces them differently)
            else if (valueToken.type == TokenType::String) {
                if (valueToken.value.has_value()) {
                    std::string stringValue = valueToken.value.value();
                    advance(); // Skip the value
                    expectToken(TokenType::Semicolon, "to end variable assignment");

                    // Add variable assignment to AST
                    ast.emplace_back(VariableAssignment{variableName, stringValue});
                } else {
                    reportError("String literal is missing its value. This appears to be a tokenizer issue.");
                }
            }
            else {
                reportError("Expected integer or string value after '=' in variable assignment.\n"
                           "   Valid examples:\n"
                           "   • let count = 42;      (integer)\n"
                           "   • let name = \"John\";   (string)\n"
                           "   Found: " + tokenTypeToString(valueToken.type));
            }
        } else {
            // No assignment, just variable declaration
            expectToken(TokenType::Semicolon, "to end variable declaration");

            // Add variable declaration to AST (without value)
            ast.emplace_back(VariableDeclaration{variableName});
        }
    }

    void parseMultiComment(AST& ast) {
        advance(); // Skip multiline comment start

        expectToken(TokenType::EndMultilineComment, "to close multiline comment. Expected '*/'");
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
                advance(); // Skip single line comments
                break;
            case TokenType::MultilineComment:
                parseMultiComment(ast);
                break;
            case TokenType::Identifier:
                // Look ahead to determine if it's a function call
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

            default:
                reportError("Unexpected " + tokenTypeToString(token.type) + " at start of statement.\n"
                           "   Expected one of: 'return', 'print', 'fn', 'let', 'asm', 'if', 'while', or identifier");
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

        try {
            // Parse all statements until end of input
            while (hasTokens()) {
                parseStatement(ast);
            }
        } catch (const std::exception& e) {
            std::cerr << "❌ Fatal parsing error: " << e.what() << std::endl;
            std::exit(1);
        }

        return ast;
    }
};

/**
 * Public parsing interface
 * Creates parser instance and delegates to internal parser
 */
AST parse(const std::vector<Token>& tokens) {
    if (tokens.empty()) {
        std::cout << "ℹ️  Note: Input is empty, returning empty AST\n";
        return AST{}; // Return empty AST for empty input
    }

    Parser parser(tokens);
    return parser.parse();
}