#pragma once
#include <variant>
#include <vector>
#include <string>

struct ReturnStatement {
    int value;
};

struct PrintStatement {
    std::variant<int, std::string> value;
    
    explicit PrintStatement(int val) : value(val) {}
    explicit PrintStatement(const std::string& val) : value(val) {}
    explicit PrintStatement(std::string&& val) : value(std::move(val)) {}
};

struct AsmStatement {
    std::string value;
};

using ASTNode = std::variant<ReturnStatement, PrintStatement, AsmStatement>;

using AST = std::vector<ASTNode>;
