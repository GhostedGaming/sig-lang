#pragma once
#include "parser_base.hpp"

class ExpressionParser : public ParserBase {
public:
    explicit ExpressionParser(const std::vector<Token>& tokens);

    // Expression parsing methods (for future use)
    // These can be expanded when adding more complex expressions
    void parseExpression(AST& ast);
    void parsePrimaryExpression(AST& ast);
    void parseBinaryExpression(AST& ast);
};
