#include <lexer/public/lexer.hpp>
#include <cctype>
#include <iostream>

static bool is_identifier_start(char c) {
    return std::isalpha(c) || c == '_';
}

static bool is_identifier_char(char c) {
    return std::isalnum(c) || c == '_';
}

static std::string read_identifier(const std::string& input, size_t& i) {
    std::string buf;
    while (i < input.size() && is_identifier_char(input[i])) {
        buf += input[i++];
    }
    return buf;
}

static std::string read_number(const std::string& input, size_t& i) {
    std::string buf;
    while (i < input.size() && std::isdigit(input[i])) {
        buf += input[i++];
    }
    return buf;
}

static void report_error(const std::string& message) {
    std::cerr << "Tokenizer error: " << message << "\n";
    std::exit(1);
}

std::vector<Token> tokenize(const std::string& input) {
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < input.size()) {
        char c = input[i];

        if (std::isspace(c)) {
            i++;
            continue;
        }

        if (is_identifier_start(c)) {
            std::string buf = read_identifier(input, i);

            if (buf == "return") {
                tokens.push_back({TokenType::KeywordReturn});
            } else if(buf == "print") {
                tokens.push_back({TokenType::KeywordPrint});
            } else {
                report_error("Unknown keyword: " + buf);
            }

        } else if (std::isdigit(c)) {
            std::string buf = read_number(input, i);
            tokens.push_back({TokenType::IntegerLiteral, buf});

        } else if (c == ';') {
            tokens.push_back({TokenType::Semicolon});
            i++;

        } else {
            report_error("Unexpected character: '" + std::string(1, c) + "'");
        }
    }

    return tokens;
}