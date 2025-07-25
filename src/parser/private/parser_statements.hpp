#pragma once
#include "parser_base.hpp"

class StatementParser : public ParserBase {
public:
    explicit StatementParser(const std::vector<Token>& tokens);

    // Statement parsing methods
    void parseReturnStatement(AST& ast);
    void parsePrintStatement(AST& ast);
    void parseAsmStatement(AST& ast);
    void parseFunctionDefinition(AST& ast);
    void parseFunctionCall(AST& ast);
    void parseVariables(AST& ast);
    void parseIfStatement(AST& ast);
    void parseWhile(AST& ast);
    void parseFor(AST& ast);
    void parseMultiComment(AST& ast);

    // Main parsing methods
    void parseStatement(AST& ast);
    void parseStatementList(AST& ast);
};
