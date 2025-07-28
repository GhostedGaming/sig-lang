#include "parser_base.hpp"
#include <iostream>
#include <filesystem>

void Parser::parseReturnStatement(AST& ast) {
    advance();
    int value;

    if (!hasTokens() || peekToken().type != TokenType::IntegerLiteral) {
        std::cerr << "Warning: Expected integer literal after 'return'. ";
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
        advance();
    }

    if (hasTokens() && peekToken().type == TokenType::Semicolon) {
        advance();
    } else {
        std::cerr << "Warning: Missing semicolon ';' after return statement. ";
        std::cerr << "All statements should end with a semicolon.\n\n";
    }

    ast.emplace_back(ReturnStatement{value});
}

void Parser::parsePrintStatement(AST& ast) {
    advance();

    expectToken(TokenType::LeftParen, "after 'print'. Syntax: print(\"hello\") or print(42)");

    if (!hasTokens()) {
        reportError("Expected content inside print statement parentheses.\n"
                   "   Examples:\n"
                   "   • print(\"Hello World\");\n"
                   "   • print(42);\n"
                   "   • print(myVariable);");
    }

    const auto& token = peekToken();

    if (token.type == TokenType::Quote) {
        advance();

        if (!hasTokens() || peekToken().type != TokenType::String) {
            reportError("Expected string content after opening quote.\n"
                       "   Example: print(\"Hello World\");");
        }

        const auto& stringToken = peekToken();
        if (stringToken.value.has_value()) {
            std::string stringValue = stringToken.value.value();
            advance();
            expectToken(TokenType::Quote, "after string content to close the string");
            expectToken(TokenType::RightParen, "after closing quote to end print statement");
            expectToken(TokenType::Semicolon, "to end print statement");
            ast.emplace_back(PrintStatement{stringValue});
        } else {
            reportError("String literal is missing its value. This appears to be a tokenizer issue.");
        }
    }
    else if (token.type == TokenType::IntegerLiteral) {
        if (token.value.has_value()) {
            int value = parseInteger(token.value.value());
            advance();
            expectToken(TokenType::RightParen, "after integer to end print statement");
            expectToken(TokenType::Semicolon, "to end print statement");
            ast.emplace_back(PrintStatement{value});
        } else {
            reportError("Integer literal is missing its value. This appears to be a tokenizer issue.");
        }
    }
    else if (token.type == TokenType::FloatLiteral) {
        if (token.value.has_value()) {
            double value = parseDouble(token.value.value());
            advance();
            expectToken(TokenType::RightParen, "after float to end print statement");
            expectToken(TokenType::Semicolon, "to end print statement");
            ast.emplace_back(PrintStatement{value});
        } else {
            reportError("Float literal is missing its value. This appears to be a tokenizer issue.");
        }
    }
    else if (token.type == TokenType::BooleanLiteral) {
        if (token.value.has_value()) {
            bool value = token.value.value() == "true";
            advance();
            expectToken(TokenType::RightParen, "after boolean to end print statement");
            expectToken(TokenType::Semicolon, "to end print statement");
            ast.emplace_back(PrintStatement{value});
        } else {
            reportError("Boolean literal is missing its value. This appears to be a tokenizer issue.");
        }
    }
    else if (token.type == TokenType::Identifier) {
        if (token.value.has_value()) {
            std::string variableName = token.value.value();
            advance();
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

void Parser::parsePrintlnStatement(AST& ast) {
    advance();

    expectToken(TokenType::LeftParen, "after 'println'. Syntax: println(\"hello\") or println(42)");

    if (!hasTokens()) {
        reportError("Expected content inside println statement parentheses.\n"
                   "   Examples:\n"
                   "   • println(\"Hello World\");\n"
                   "   • println(42);\n"
                   "   • println(myVariable);");
    }

    const auto& token = peekToken();

    if (token.type == TokenType::Quote) {
        advance();

        if (!hasTokens() || peekToken().type != TokenType::String) {
            reportError("Expected string content after opening quote.\n"
                       "   Example: println(\"Hello World\");");
        }

        const auto& stringToken = peekToken();
        if (stringToken.value.has_value()) {
            std::string stringValue = stringToken.value.value();
            advance();
            expectToken(TokenType::Quote, "after string content to close the string");
            expectToken(TokenType::RightParen, "after closing quote to end println statement");
            expectToken(TokenType::Semicolon, "to end println statement");
            ast.emplace_back(PrintlnStatement{stringValue});
        } else {
            reportError("String literal is missing its value. This appears to be a tokenizer issue.");
        }
    }
    else if (token.type == TokenType::IntegerLiteral) {
        if (token.value.has_value()) {
            int value = parseInteger(token.value.value());
            advance();
            expectToken(TokenType::RightParen, "after integer to end println statement");
            expectToken(TokenType::Semicolon, "to end println statement");
            ast.emplace_back(PrintlnStatement{value});
        } else {
            reportError("Integer literal is missing its value. This appears to be a tokenizer issue.");
        }
    }
    else if (token.type == TokenType::FloatLiteral) {
        if (token.value.has_value()) {
            double value = parseDouble(token.value.value());
            advance();
            expectToken(TokenType::RightParen, "after float to end println statement");
            expectToken(TokenType::Semicolon, "to end println statement");
            ast.emplace_back(PrintlnStatement{value});
        } else {
            reportError("Float literal is missing its value. This appears to be a tokenizer issue.");
        }
    }
    else if (token.type == TokenType::BooleanLiteral) {
        if (token.value.has_value()) {
            bool value = token.value.value() == "true";
            advance();
            expectToken(TokenType::RightParen, "after boolean to end println statement");
            expectToken(TokenType::Semicolon, "to end println statement");
            ast.emplace_back(PrintlnStatement{value});
        } else {
            reportError("Boolean literal is missing its value. This appears to be a tokenizer issue.");
        }
    }
    else if (token.type == TokenType::Identifier) {
        if (token.value.has_value()) {
            std::string variableName = token.value.value();
            advance();
            expectToken(TokenType::RightParen, "after variable name to end println statement");
            expectToken(TokenType::Semicolon, "to end println statement");
            ast.emplace_back(PrintVariable{variableName});
        } else {
            reportError("Variable name is missing its value. This appears to be a tokenizer issue.");
        }
    }
    else {
        reportError("Expected string, integer, or variable name inside println statement.\n"
                   "   Valid examples:\n"
                   "   • println(\"Hello\");     (string)\n"
                   "   • println(42);           (integer)\n"
                   "   • println(myVar);        (variable)\n"
                   "   Found: " + tokenTypeToString(token.type));
    }
}

void Parser::parseAsmStatement(AST& ast) {
    advance();

    expectToken(TokenType::LeftParen, "after 'asm'. Syntax: asm(\"mov eax, 1\");");
    expectToken(TokenType::Quote, "to start assembly string. Example: asm(\"nop\");");

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

void Parser::parseVariables(AST& ast) {
    advance();

    if (!hasTokens() || peekToken().type != TokenType::Identifier) {
        reportError("Expected variable name after 'let' keyword.\n"
                   "   Examples:\n"
                   "   • let myVar;\n"
                   "   • let count = 42;\n"
                   "   • let name = \"John\";");
    }

    const std::string variableName = peekToken().value.value_or("unnamed");
    advance();

    // Check for optional type annotation
    std::optional<SigType> typeAnnotation;
    if (hasTokens() && peekToken().type == TokenType::Colon) {
        advance(); // Skip colon
        typeAnnotation = parseTypeAnnotation();
    }

    if (hasTokens() && peekToken().type == TokenType::Equal) {
        advance();

        if (!hasTokens()) {
            reportError("Expected value after '=' in variable assignment.\n"
                       "   Examples:\n"
                       "   • let x = 42;\n"
                       "   • let text = \"hello\";");
        }

        const auto& valueToken = peekToken();

        if (valueToken.type == TokenType::HexLiteral) {
            if (valueToken.value.has_value()) {
                uint64_t value = parseHexLiteral(valueToken.value.value());
                advance();
                expectToken(TokenType::Semicolon, "to end variable assignment");

                if (typeAnnotation.has_value()) {
                    TypedValue typedValue = createTypedValue(typeAnnotation.value(), value);
                    ast.emplace_back(VariableAssignment{variableName, typedValue, typeAnnotation});
                } else {
                    // Default to u32 for hex literals without explicit type
                    TypedValue typedValue = createTypedValue(SigType::U32, value);
                    ast.emplace_back(VariableAssignment{variableName, typedValue, SigType::U32});
                }
            } else {
                reportError("Hex literal is missing its value. This appears to be a tokenizer issue.");
            }
        }
        else if (valueToken.type == TokenType::IntegerLiteral) {
            if (valueToken.value.has_value()) {
                int value = parseInteger(valueToken.value.value());
                advance();
                expectToken(TokenType::Semicolon, "to end variable assignment");

                if (typeAnnotation.has_value()) {
                    TypedValue typedValue = createTypedValue(typeAnnotation.value(), static_cast<uint64_t>(value));
                    ast.emplace_back(VariableAssignment{variableName, typedValue, typeAnnotation});
                } else {
                    ast.emplace_back(VariableAssignment{variableName, value, std::nullopt});
                }
            } else {
                reportError("Integer literal is missing its value. This appears to be a tokenizer issue.");
            }
        }
        else if (valueToken.type == TokenType::FloatLiteral) {
            if (valueToken.value.has_value()) {
                double value = parseDouble(valueToken.value.value());
                advance();
                expectToken(TokenType::Semicolon, "to end variable assignment");

                ast.emplace_back(VariableAssignment{variableName, value, std::nullopt});
            } else {
                reportError("Float literal is missing its value. This appears to be a tokenizer issue.");
            }
        }
        else if (valueToken.type == TokenType::BooleanLiteral) {
            if (valueToken.value.has_value()) {
                bool value = valueToken.value.value() == "true";
                advance();
                expectToken(TokenType::Semicolon, "to end variable assignment");

                ast.emplace_back(VariableAssignment{variableName, value, std::nullopt});
            } else {
                reportError("Boolean literal is missing its value. This appears to be a tokenizer issue.");
            }
        }
        else if (valueToken.type == TokenType::Quote) {
            advance();

            if (!hasTokens() || peekToken().type != TokenType::String) {
                reportError("Expected string content after opening quote in variable assignment.\n"
                           "   Example: let name = \"John\";");
            }

            const auto& stringToken = peekToken();
            if (stringToken.value.has_value()) {
                std::string stringValue = stringToken.value.value();
                advance();
                expectToken(TokenType::Quote, "to close string in variable assignment");
                expectToken(TokenType::Semicolon, "to end variable assignment");

                ast.emplace_back(VariableAssignment{variableName, stringValue});
            } else {
                reportError("String literal is missing its value. This appears to be a tokenizer issue.");
            }
        }
        else if (valueToken.type == TokenType::String) {
            if (valueToken.value.has_value()) {
                std::string stringValue = valueToken.value.value();
                advance();
                expectToken(TokenType::Semicolon, "to end variable assignment");

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
        expectToken(TokenType::Semicolon, "to end variable declaration");

        ast.emplace_back(VariableDeclaration{variableName, typeAnnotation});
    }
}

void Parser::parseModStatement(AST& ast) {
    advance();

    if (hasTokens() && peekToken().type == TokenType::Quote) {
        advance();

        if (!hasTokens() || peekToken().type != TokenType::String) {
            reportError("Expected module filename after opening quote.\n"
                       "   Example: mod \"filename.sg\";");
        }

        const auto& stringToken = peekToken();
        if (stringToken.value.has_value()) {
            std::string moduleFile = stringToken.value.value();
            advance();
            expectToken(TokenType::Quote, "to close module filename");
            expectToken(TokenType::Semicolon, "to end module statement");
            
            std::filesystem::path currentFile(current_file_path);
            std::filesystem::path moduleDir = currentFile.parent_path();
            std::filesystem::path fullModulePath = moduleDir / moduleFile;
            
            ast.emplace_back(ModStatement{fullModulePath.string()});

            if (std::filesystem::exists(fullModulePath)) {
                // Module exists, good
            } else {
                reportError("Module file does not exist: " + fullModulePath.string());
            }
        }
    }
}

void Parser::parseMultiComment(AST& ast) {
    advance();
    expectToken(TokenType::EndMultilineComment, "to close multiline comment. Expected '*/'");
}
