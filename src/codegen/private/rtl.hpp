#pragma once
#include <vector>
#include <string>
#include <unordered_map>

// Enhanced RTL instruction with more semantic information
struct RTLInsn {
    enum OpType { 
        SET, CALL, RETURN, LABEL, DATA, INLINE_ASM, LOAD, STORE,
        ADD, SUB, MUL, IMUL, DIV, IDIV, MOV, CMP, JMP, JE, JNE,
        PUSH, POP, LEA, SYSCALL, XOR, AND, OR, SHL, SHR
    };
    
    OpType op;
    std::vector<std::string> operands;
    std::unordered_map<std::string, std::string> attributes;
    std::string result_reg; // For register allocation
    std::vector<std::string> input_regs; // Dependencies
    
    RTLInsn(OpType operation, std::vector<std::string> ops = {}) 
        : op(operation), operands(std::move(ops)) {}
};
