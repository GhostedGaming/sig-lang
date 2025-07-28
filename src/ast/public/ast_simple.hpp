#pragma once
#include <variant>
#include <vector>
#include <string>
#include <optional>
#include <cstdint>

// Type system definitions
enum class SigType {
    Untyped,    // For backward compatibility
    U8, U16, U32, U64,
    I8, I16, I32, I64,
    Bool,
    Float,
    String,
    Pointer     // Pointer type
};

// Typed value that can hold different integer types
struct TypedValue {
    SigType type;
    std::variant<
        uint8_t, uint16_t, uint32_t, uint64_t,  // Unsigned integers
        int8_t, int16_t, int32_t, int64_t,      // Signed integers
        bool, double, std::string               // Other types
    > value;
};

struct ReturnStatement {
    int value;
};

enum class SigBinaryOperator {
    Add,
    Subtract,
    Multiply,
    Divide,
    Modulo,
    // Comparison operators
    Equal,
    NotEqual,
    LessThan,
    LessThanEqual,
    GreaterThan,
    GreaterThanEqual,
    // Logical operators
    And,
    Or,
    Not,
    // Bitwise operators
    BitwiseAnd,
    BitwiseOr,
    BitwiseXor,
    LeftShift,
    RightShift
};

// Simple expression type - just basic values for now
using Expression = std::variant<int, double, bool, std::string, TypedValue>;

struct BinaryExpression {
    Expression left;
    SigBinaryOperator operator_type;
    Expression right;
};

struct UnaryExpression {
    SigBinaryOperator operator_type; // For NOT operator
    Expression operand;
};

struct PrintStatement {
    Expression value;
};

struct PrintlnStatement {
    Expression value;
};

struct AsmStatement {
    std::string value;
};

struct FunctionCall {
    std::string function_name;
    std::vector<Expression> arguments;
};

struct VariableDeclaration {
    std::string var_name;
    std::optional<SigType> type;  // Optional type annotation
};

struct VariableAssignment {
    std::string var_name;
    Expression value;
    std::optional<SigType> type;  // Optional type annotation for declaration
};

struct PrintVariable {
    std::string variableName;
};

struct ModStatement {
    std::string filename;
};

// Forward declarations for control flow
struct IfStatement;
struct WhileStatement;
struct ForStatement;
struct FunctionDefinition;

// Main ASTNode type
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
    BinaryExpression,
    UnaryExpression,
    IfStatement,
    WhileStatement,
    ForStatement
>;

// ElifClause for if-else if chains
struct ElifClause {
    std::string left;
    std::string op;
    std::string right;
    std::vector<ASTNode> block;
};

// IfStatement structure with elif support
struct IfStatement {
    std::string left;
    std::string op;
    std::string right;
    std::vector<ASTNode> thenBlock;
    std::vector<ElifClause> elifClauses;
    std::optional<std::vector<ASTNode>> elseBlock;
};

struct WhileStatement {
    std::string left;
    std::string op;
    std::string right;
    std::vector<ASTNode> body;
};

struct FunctionDefinition {
    std::string name;
    std::vector<std::string> params;
    std::vector<ASTNode> body;
};

struct ForStatement {
    std::string initialization;
    std::string condition;
    std::string count;
    std::vector<ASTNode> body;
};

using AST = std::vector<ASTNode>;
