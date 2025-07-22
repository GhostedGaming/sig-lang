#pragma once
#include <variant>
#include <vector>
#include <string>
#include <memory>
#include <optional>

// AST node type definitions
struct ReturnStatement {
    int value;
};

struct PrintStatement {
    std::variant<int, std::string> value;
};

struct AsmStatement {
    std::string value;
};

struct FunctionCall {
    std::string function_name;
};

struct VariableDeclaration {
    std::string var_name;
};

struct VariableAssignment {
    std::string var_name;
    std::variant<int, std::string, double, float> value;
};

struct PrintVariable {
    std::string variableName;
};

// Forward declare ASTNode for FunctionDefinition
using ASTNode = std::variant<
    ReturnStatement,
    PrintStatement,
    AsmStatement,
    struct FunctionDefinition,
    FunctionCall,
    VariableDeclaration,
    VariableAssignment,
    PrintVariable
>;

using AST = std::vector<ASTNode>;

// Now define FunctionDefinition with complete AST type
struct FunctionDefinition {
    std::string name;
    AST body;
};