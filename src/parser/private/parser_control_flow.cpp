#include "parser_base.hpp"
#include <iostream>

void Parser::parseIfStatement(AST& ast) {
    advance();

    expectToken(TokenType::LeftParen, "after 'if' keyword. Syntax: if (condition) { ... }");

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
    expectToken(TokenType::LeftBrace, "after if condition. Expected opening '{'");
    
    parseStatementList(ifStmt.thenBlock);
    
    expectToken(TokenType::RightBrace, "after if then block. Expected closing '}'");

    while (hasTokens() && peekToken().type == TokenType::KeywordElif) {
        advance();
        
        ElifClause elifClause;
        
        expectToken(TokenType::LeftParen, "after 'elif' keyword. Expected opening '('");
        
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
        
        parseStatementList(elifClause.block);
        
        expectToken(TokenType::RightBrace, "after elif block. Expected closing '}'");
        
        ifStmt.elifClauses.push_back(elifClause);
    }
    
    if (hasTokens() && peekToken().type == TokenType::KeywordElse) {
        advance();
        expectToken(TokenType::LeftBrace, "after else keyword. Expected opening '{'");
        
        ifStmt.elseBlock = std::vector<ASTNode>();
        
        parseStatementList(ifStmt.elseBlock.value());
        
        expectToken(TokenType::RightBrace, "after else block. Expected closing '}'");
    }
    
    ast.push_back(ifStmt);
}

void Parser::parseWhile(AST& ast) {
    advance();
    expectToken(TokenType::LeftParen, "after while keyword. Expected opening '('");

    if (!hasTokens()) {
        reportError("Expected condition after '(' in while statement");
        return;
    }
    
    WhileStatement whileStmt;
    
    Token firstToken = peekToken();
    if (firstToken.type != TokenType::Identifier && 
        firstToken.type != TokenType::IntegerLiteral && 
        firstToken.type != TokenType::String) {
        reportError("Expected identifier, number, or string in while condition");
        return;
    }
    
    whileStmt.left = firstToken.value.value_or("");
    advance();
    
    if (hasTokens() && peekToken().type != TokenType::RightParen) {
        Token opToken = peekToken();
        
        bool isComparisonOp = (opToken.type == TokenType::EqualEqual ||
                             opToken.type == TokenType::NotEqual ||
                             opToken.type == TokenType::LessThan ||
                             opToken.type == TokenType::LessThanEqual ||
                             opToken.type == TokenType::GreaterThan ||
                             opToken.type == TokenType::GreaterThanEqual);
        
        if (isComparisonOp) {
            switch (opToken.type) {
                case TokenType::EqualEqual: whileStmt.op = "=="; break;
                case TokenType::NotEqual: whileStmt.op = "!="; break;
                case TokenType::LessThan: whileStmt.op = "<"; break;
                case TokenType::LessThanEqual: whileStmt.op = "<="; break;
                case TokenType::GreaterThan: whileStmt.op = ">"; break;
                case TokenType::GreaterThanEqual: whileStmt.op = ">="; break;
                default: break;
            }
            advance();
            
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
        whileStmt.op = "";
        whileStmt.right = "";
    }
    
    expectToken(TokenType::RightParen, "after while condition. Expected closing ')'");
    expectToken(TokenType::LeftBrace, "after while condition. Expected opening '{'");
    
    parseStatementList(whileStmt.body);
    
    expectToken(TokenType::RightBrace, "after while body. Expected closing '}'");
    
    ast.push_back(whileStmt);
}

void Parser::parseFor(AST& ast) {
    advance();
    expectToken(TokenType::LeftParen, "expected '(' after for");

    if (!hasTokens()) {
        reportError("Expected initializer in for loop after '('");
        return;
    }

    ForStatement forstmnt;

    Token initialization = peekToken();
    if (initialization.type == TokenType::Identifier || initialization.type == TokenType::IntegerLiteral) {
        forstmnt.initialization = initialization.value.value_or("");
        advance();
    } else {
        reportError("Expected identifier or number as initializer in for loop");
        return;
    }

    expectToken(TokenType::Comma, "Expected ',' after initializer");

    if (!hasTokens()) {
        reportError("Expected condition in for loop after ','");
        return;
    }

    Token condition = peekToken();
    if (condition.type == TokenType::Identifier || condition.type == TokenType::IntegerLiteral) {
        forstmnt.condition = condition.value.value_or("");
        advance();
    } else {
        reportError("Expected identifier or number as condition in for loop");
        return;
    }

    expectToken(TokenType::Comma, "Expected ',' after condition");

    if (!hasTokens()) {
        reportError("Expected count/increment in for loop after ','");
        return;
    }

    Token count = peekToken();
    if (count.type == TokenType::Identifier || count.type == TokenType::IntegerLiteral) {
        forstmnt.count = count.value.value_or("");
        advance();
    } else {
        reportError("Expected identifier or number as count in for loop");
        return;
    }

    expectToken(TokenType::RightParen, "Expected ')' after for loop header");
    expectToken(TokenType::LeftBrace, "Expected '{' to start for loop body");

    parseStatementList(forstmnt.body);

    expectToken(TokenType::RightBrace, "Expected '}' to close for loop body");

    ast.push_back(forstmnt);
}
