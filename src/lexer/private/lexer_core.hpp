#pragma once
#include "../public/token.hpp"
#include "lexer_helpers.hpp"
#include <vector>
#include <string>

class LexerCore {
private:
    std::vector<Token> tokens;
    size_t position;
    size_t size;
    const char* data;
    const std::string* input_ref;

    // Token creation methods
    void process_identifier();
    void process_integer();
    void process_string();
    void process_single_char_operators();
    void process_double_char_operators();
    void process_comments();

public:
    explicit LexerCore(const std::string& input);
    std::vector<Token> tokenize();
};
