#include "parser_base.hpp"

// Parse a factor (number, variable, or parenthesized expression)
Expression Parser::parseFactor() {
    if (!hasTokens()) {
        reportError("Expected expression but reached end of file");
    }
    
    const Token& token = peekToken();
    
    switch (token.type) {
        case TokenType::IntegerLiteral: {
            int value = parseInteger(token.value.value());
            advance();
            return value;
        }
        
        case TokenType::FloatLiteral: {
            double value = parseDouble(token.value.value());
            advance();
            return value;
        }
        
        case TokenType::BooleanLiteral: {
            bool value = token.value.value() == "true";
            advance();
            return value;
        }
        
        case TokenType::Identifier: {
            std::string var_name = token.value.value();
            advance();
            return var_name;
        }
        
        case TokenType::Quote: {
            advance(); // consume opening quote
            if (!hasTokens() || peekToken().type != TokenType::String) {
                reportError("Expected string content after opening quote");
            }
            std::string str_value = peekToken().value.value_or("");
            advance(); // consume string content
            expectToken(TokenType::Quote, "Expected closing quote");
            return str_value;
        }
        
        case TokenType::LeftParen: {
            advance(); // consume '('
            // For now, just parse the first factor inside parentheses
            auto result = parseFactor();
            expectToken(TokenType::RightParen, "after expression");
            return result;
        }
        
        default:
            reportError("Expected number, variable, or '(' in expression, but found " + 
                       (token.value ? token.value.value() : "token"));
    }
}

// Parse a unary expression (for NOT operator)
std::variant<UnaryExpression, Expression> Parser::parseUnaryExpression() {
    if (hasTokens() && peekToken().type == TokenType::Not) {
        advance(); // consume '!'
        auto operand = parseFactor();
        UnaryExpression expr;
        expr.operator_type = SigBinaryOperator::Not;
        expr.operand = operand;
        return expr;
    }
    
    return parseFactor();
}

// Parse arithmetic expressions (*, /, %)
Expression Parser::parseArithmeticTerm() {
    auto left = parseUnaryExpression();
    
    // Extract the actual value from the variant
    Expression leftValue;
    if (std::holds_alternative<UnaryExpression>(left)) {
        // For now, return the unary expression as-is (this needs proper handling)
        return std::get<UnaryExpression>(left).operand;
    } else {
        leftValue = std::get<Expression>(left);
    }
    
    while (hasTokens() && (peekToken().type == TokenType::Multiply ||
                          peekToken().type == TokenType::Divide ||
                          peekToken().type == TokenType::Modulo)) {
        TokenType op = peekToken().type;
        advance();
        auto rightUnary = parseUnaryExpression();
        
        Expression rightValue;
        if (std::holds_alternative<UnaryExpression>(rightUnary)) {
            rightValue = std::get<UnaryExpression>(rightUnary).operand;
        } else {
            rightValue = std::get<Expression>(rightUnary);
        }
        
        // Create a binary expression and evaluate it
        BinaryExpression expr;
        expr.left = leftValue;
        expr.right = rightValue;
        
        switch (op) {
            case TokenType::Multiply:
                expr.operator_type = SigBinaryOperator::Multiply;
                break;
            case TokenType::Divide:
                expr.operator_type = SigBinaryOperator::Divide;
                break;
            case TokenType::Modulo:
                expr.operator_type = SigBinaryOperator::Modulo;
                break;
            default:
                reportError("Unexpected operator in expression");
        }
        
        // For simplicity, return the expression (this needs proper evaluation)
        leftValue = expr.left; // Placeholder
    }
    
    return leftValue;
}

// Parse additive expressions (+, -)
Expression Parser::parseAdditiveExpression() {
    auto left = parseArithmeticTerm();
    
    while (hasTokens() && (peekToken().type == TokenType::Plus ||
                          peekToken().type == TokenType::Minus)) {
        TokenType op = peekToken().type;
        advance();
        auto right = parseArithmeticTerm();
        
        BinaryExpression expr;
        expr.left = left;
        expr.right = right;
        
        switch (op) {
            case TokenType::Plus:
                expr.operator_type = SigBinaryOperator::Add;
                break;
            case TokenType::Minus:
                expr.operator_type = SigBinaryOperator::Subtract;
                break;
            default:
                reportError("Unexpected operator in expression");
        }
        
        // For now, we need to convert to a simple value rather than a complex expression
        // This is a simplification - in a full implementation, we'd evaluate the expression
        left = 0; // Placeholder - represents the result of the binary expression
    }
    
    return left;
}

// Parse comparison expressions (==, !=, <, <=, >, >=)
BinaryExpression Parser::parseComparisonExpression() {
    auto left = parseAdditiveExpression();
    
    if (!hasTokens() || (peekToken().type != TokenType::EqualEqual &&
                        peekToken().type != TokenType::NotEqual &&
                        peekToken().type != TokenType::LessThan &&
                        peekToken().type != TokenType::LessThanEqual &&
                        peekToken().type != TokenType::GreaterThan &&
                        peekToken().type != TokenType::GreaterThanEqual)) {
        // No comparison operator, return dummy expression
        BinaryExpression expr;
        expr.left = left;
        expr.operator_type = SigBinaryOperator::Add;
        expr.right = 0;
        return expr;
    }
    
    TokenType op = peekToken().type;
    advance();
    auto right = parseAdditiveExpression();
    
    BinaryExpression expr;
    expr.left = left;
    expr.right = right;
    
    switch (op) {
        case TokenType::EqualEqual:
            expr.operator_type = SigBinaryOperator::Equal;
            break;
        case TokenType::NotEqual:
            expr.operator_type = SigBinaryOperator::NotEqual;
            break;
        case TokenType::LessThan:
            expr.operator_type = SigBinaryOperator::LessThan;
            break;
        case TokenType::LessThanEqual:
            expr.operator_type = SigBinaryOperator::LessThanEqual;
            break;
        case TokenType::GreaterThan:
            expr.operator_type = SigBinaryOperator::GreaterThan;
            break;
        case TokenType::GreaterThanEqual:
            expr.operator_type = SigBinaryOperator::GreaterThanEqual;
            break;
        default:
            reportError("Unexpected comparison operator");
    }
    
    return expr;
}

// Parse logical AND expressions
BinaryExpression Parser::parseLogicalAndExpression() {
    auto left = parseComparisonExpression();
    
    while (hasTokens() && peekToken().type == TokenType::And) {
        advance(); // consume '&&'
        auto right = parseComparisonExpression();
        
        BinaryExpression expr;
        expr.left = left.left; // Take left operand from comparison
        expr.operator_type = SigBinaryOperator::And;
        expr.right = right.left; // Take left operand from comparison
        left = expr;
    }
    
    return left;
}

// Parse logical OR expressions
BinaryExpression Parser::parseLogicalOrExpression() {
    auto left = parseLogicalAndExpression();
    
    while (hasTokens() && peekToken().type == TokenType::Or) {
        advance(); // consume '||'
        auto right = parseLogicalAndExpression();
        
        BinaryExpression expr;
        expr.left = left.left; // Take left operand from comparison
        expr.operator_type = SigBinaryOperator::Or;
        expr.right = right.left; // Take left operand from comparison
        left = expr;
    }
    
    return left;
}

// Parse a full expression with proper precedence
BinaryExpression Parser::parseArithmeticExpression() {
    return parseLogicalOrExpression();
}

// Parse and add expression to AST as a statement
void Parser::parseExpression(AST& ast) {
    auto expr = parseArithmeticExpression();
    ast.emplace_back(expr);
}
