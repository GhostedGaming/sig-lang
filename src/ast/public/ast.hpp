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

struct IfStatement {
    std::string left;
    std::string op;
    std::string right;
    std::vector<std::variant<
        ReturnStatement,
        PrintStatement,
        AsmStatement,
        struct FunctionDefinition,
        FunctionCall,
        VariableDeclaration,
        VariableAssignment,
        PrintVariable,
        IfStatement
    >> thenBlock;
    std::optional<std::vector<std::variant<
        ReturnStatement,
        PrintStatement,
        AsmStatement,
        struct FunctionDefinition,
        FunctionCall,
        VariableDeclaration,
        VariableAssignment,
        PrintVariable,
        IfStatement
    >>> elseBlock;
};

struct FunctionDefinition {
    std::string name;
    std::vector<std::variant<
        ReturnStatement,
        PrintStatement,
        AsmStatement,
        struct FunctionDefinition,
        FunctionCall,
        VariableDeclaration,
        VariableAssignment,
        PrintVariable,
        IfStatement
    >> body;
};

// Final ASTNode and AST typedefs
using ASTNode = std::variant<
    ReturnStatement,
    PrintStatement,
    AsmStatement,
    FunctionDefinition,
    FunctionCall,
    VariableDeclaration,
    VariableAssignment,
    PrintVariable,
    IfStatement
>;

using AST = std::vector<ASTNode>;