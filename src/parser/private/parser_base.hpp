#pragma once
#include "../public/parser.hpp"
#include <lexer/public/token.hpp>
#include <ast/public/ast_simple.hpp>
#include <string>
#include <vector>
#include <string_view>

class Parser {
private:
    const std::vector<Token>& tokens;
    size_t current;
    const size_t size;
    std::string current_file_path;

    int parseInteger(std::string_view str) const;
    double parseDouble(std::string_view str) const;
    uint64_t parseHexLiteral(std::string_view str) const;
    SigType parseTypeAnnotation();
    TypedValue createTypedValue(SigType type, uint64_t value) const;
    std::string getErrorContext() const;
    void skipToRecoveryPoint();
    std::string getSuggestions() const;
    [[noreturn]] void reportError(const std::string& message) const;
    void reportErrorWithRecovery(const std::string& message);
    [[noreturn]] void reportExpectedError(TokenType expected, const std::string& context = "") const;

    bool hasTokens(size_t count = 1) const;
    const Token& peekToken(size_t offset = 0) const;
    void advance(size_t count = 1);
    void expectToken(TokenType expected, const std::string& context = "");

public:
    explicit Parser(const std::vector<Token>& tokens, const std::string& file_path = "");

    void parseReturnStatement(AST& ast);
    void parsePrintStatement(AST& ast);
    void parsePrintlnStatement(AST& ast);
    void parseFor(AST& ast);
    void parseFunctionDefinition(AST& ast);
    void parseFunctionCall(AST& ast);
    void parseAsmStatement(AST& ast);
    void parseIfStatement(AST& ast);
    void parseWhile(AST& ast);
    void parseVariables(AST& ast);
    void parseMultiComment(AST& ast);
    void parseStatementList(AST& ast);
    void parseStatement(AST& ast);
    void parseModStatement(AST& ast);
    void parseExpression(AST& ast);
    BinaryExpression parseArithmeticExpression();
    Expression parseFactor();
    std::variant<UnaryExpression, Expression> parseUnaryExpression();
    Expression parseArithmeticTerm();
    Expression parseAdditiveExpression();
    BinaryExpression parseComparisonExpression();
    BinaryExpression parseLogicalAndExpression();
    BinaryExpression parseLogicalOrExpression();
    AST parse();
};
