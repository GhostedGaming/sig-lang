#include "parser_base.hpp"
#include <iostream>

void Parser::parseFunctionDefinition(AST& ast) {
    bool isPublic = false;
    if (hasTokens() && peekToken().type == TokenType::KeywordPub) {
        isPublic = true;
        advance();
    }
    
    expectToken(TokenType::Function, "when defining a function. Use: fn functionName() { ... }");
    
    if (!hasTokens() || peekToken().type != TokenType::Identifier) {
        reportError("Expected function name after 'fn' keyword.\n"
                   " Example: fn myFunction() { return 0; }");
    }
    const std::string functionName = peekToken().value.value_or("unnamed");
    advance();
    
    expectToken(TokenType::LeftParen, "expected '(' after function name");
    
    std::vector<std::string> functionParams;
    
    if (hasTokens() && peekToken().type != TokenType::RightParen) {
        do {
            if (!hasTokens() || peekToken().type != TokenType::Identifier) {
                reportError("Expected parameter name in function parameter list.\n"
                           " Example: fn myFunction(param1, param2) { ... }");
            }
            
            std::string paramName = peekToken().value.value_or("unnamed_param");
            functionParams.push_back(paramName);
            advance();
            
            if (hasTokens() && peekToken().type == TokenType::Comma) {
                advance();
                if (!hasTokens() || peekToken().type == TokenType::RightParen) {
                    reportError("Expected parameter name after comma in parameter list");
                }
            } else {
                break;
            }
        } while (hasTokens() && peekToken().type != TokenType::RightParen);
    }
    
    expectToken(TokenType::RightParen, "expected ')' to close function parameter list");
    expectToken(TokenType::LeftBrace, "to start function body. Example: fn test() { return 0; }");
    
    AST functionBody;
    parseStatementList(functionBody);
    
    expectToken(TokenType::RightBrace, "to close function body");
    
    ast.emplace_back(FunctionDefinition{functionName, functionParams, std::move(functionBody)});
}

void Parser::parseFunctionCall(AST& ast) {
    if (!hasTokens() || peekToken().type != TokenType::Identifier) {
        reportError("Expected function name for function call.\n"
                   "   Example: myFunction();");
    }

    const std::string functionName = peekToken().value.value_or("unnamed");
    advance();

    expectToken(TokenType::LeftParen, "after function name. Example: " + functionName + "();");
    
    // For now, skip any parameters until we get to the closing paren
    // TODO: Properly parse and handle function call arguments
    while (hasTokens() && peekToken().type != TokenType::RightParen) {
        advance();
    }
    
    expectToken(TokenType::RightParen, "to close function call");
    expectToken(TokenType::Semicolon, "to end function call statement");

    ast.emplace_back(FunctionCall{functionName});
}
