#pragma once
#include <variant>
#include <vector>

struct ReturnStatement {
    int value;
};

struct PrintStatement {
    int value;
};

using ASTNode = std::variant<ReturnStatement, PrintStatement>;

using AST = std::vector<ASTNode>;
