#pragma once
#include <variant>
#include <vector>
#include <string>
#include <memory>
#include <optional>

struct ReturnStatement {
    int value;
};

struct PrintStatement {
    std::variant<int, std::string> value;
};

struct PrintlnStatement {
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

struct ModStatement {
    std::string filename;
};

struct ElifClause {
    std::string left;
    std::string op;
    std::string right;
    std::vector<std::variant<
        ReturnStatement,
        PrintStatement,
        PrintlnStatement,
        AsmStatement,
        struct FunctionDefinition,
        FunctionCall,
        VariableDeclaration,
        VariableAssignment,
        PrintVariable,
        ModStatement,
        struct IfStatement,
        struct WhileStatement,
        struct ForStatement
    >> block;
};

struct IfStatement {
    std::string left;
    std::string op;
    std::string right;
    std::vector<std::variant<
        ReturnStatement,
        PrintStatement,
        PrintlnStatement,
        AsmStatement,
        struct FunctionDefinition,
        FunctionCall,
        VariableDeclaration,
        VariableAssignment,
        PrintVariable,
        ModStatement,
        IfStatement,
        struct WhileStatement,
        struct ForStatement
    >> thenBlock;
    std::vector<ElifClause> elifClauses;
    std::optional<std::vector<std::variant<
        ReturnStatement,
        PrintStatement,
        PrintlnStatement,
        AsmStatement,
        struct FunctionDefinition,
        FunctionCall,
        VariableDeclaration,
        VariableAssignment,
        PrintVariable,
        ModStatement,
        IfStatement,
        struct WhileStatement,
        struct ForStatement
    >>> elseBlock;
};

struct WhileStatement {
    std::string left;
    std::string op;
    std::string right;
    std::vector<std::variant<
        ReturnStatement,
        PrintStatement,
        PrintlnStatement,
        AsmStatement,
        struct FunctionDefinition,
        FunctionCall,
        VariableDeclaration,
        VariableAssignment,
        PrintVariable,
        ModStatement,
        IfStatement,
        struct WhileStatement,
        struct ForStatement
    >> body;
};

struct FunctionDefinition {
    std::string name;
    std::vector<std::string> params;
    std::vector<std::variant<
        ReturnStatement,
        PrintStatement,
        PrintlnStatement,
        AsmStatement,
        struct FunctionDefinition,
        FunctionCall,
        VariableDeclaration,
        VariableAssignment,
        PrintVariable,
        ModStatement,
        IfStatement,
        struct WhileStatement,
        struct ForStatement
    >> body;
};

struct ForStatement {
    std::string initialization;
    std::string condition;
    std::string count;
    std::vector<std::variant<
        ReturnStatement,
        PrintStatement,
        PrintlnStatement,
        AsmStatement,
        struct FunctionDefinition,
        FunctionCall,
        VariableDeclaration,
        VariableAssignment,
        PrintVariable,
        ModStatement,
        IfStatement,
        struct WhileStatement,
        struct ForStatement
    >> body;
};

using ASTNode = std::variant<
    ReturnStatement,
    PrintStatement,
    PrintlnStatement,
    AsmStatement,
    FunctionDefinition,
    FunctionCall,
    VariableDeclaration,
    VariableAssignment,
    PrintVariable,
    ModStatement,
    IfStatement,
    WhileStatement,
    ForStatement
>;

using AST = std::vector<ASTNode>;