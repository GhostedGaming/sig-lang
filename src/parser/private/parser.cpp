#include <parser/public/parser.hpp>
#include <iostream>
#include <cstdlib>

AST parse(const std::vector<Token>& tokens) {
    AST ast;
    size_t i = 0;

    while (i < tokens.size()) {
        if (tokens[i].type != TokenType::KeywordReturn) {
            std::cerr << "Expected 'return'\n";
            std::exit(1);
        }

        if (i + 1 >= tokens.size() || tokens[i + 1].type != TokenType::IntegerLiteral) {
            std::cerr << "Expected integer literal after 'return'\n";
            std::exit(1);
        }

        if (i + 2 >= tokens.size() || tokens[i + 2].type != TokenType::Semicolon) {
            std::cerr << "Expected semicolon after integer\n";
            std::exit(1);
        }

        int value = std::stoi(tokens[i + 1].value.value());
        ast.push_back(ReturnStatement{value});

        i += 3;
    }

    return ast;
}
