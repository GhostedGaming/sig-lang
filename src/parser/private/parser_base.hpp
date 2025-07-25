#pragma once
#include "../public/parser.hpp"
#include <lexer/public/token.hpp>
#include <string>
#include <vector>

class ParserBase {
protected:
    const std::vector<Token>& tokens;
    size_t current;
    size_t size;

    // Core navigation methods
    inline bool hasTokens(size_t count = 1) const;
    inline const Token& peekToken(size_t offset = 0) const;
    inline void advance(size_t count = 1);
    inline void expectToken(TokenType expected, const std::string& context = "");

    // Error handling
    [[noreturn]] void reportError(const std::string& message) const;
    [[noreturn]] void reportExpectedError(TokenType expected, const std::string& context = "") const;
    void reportErrorWithRecovery(const std::string& message);

    // Helper methods
    std::string getErrorContext() const;
    std::string getSuggestions() const;
    void skipToRecoveryPoint();
    int parseInteger(const std::string& str) const;

public:
    explicit ParserBase(const std::vector<Token>& tokens);
};
