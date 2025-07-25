#include "../public/lexer.hpp"
#include "lexer_core.hpp"

std::vector<Token> tokenize(const std::string& input) {
    LexerCore lexer(input);
    return lexer.tokenize();
}
