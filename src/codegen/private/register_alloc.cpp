#include "register_alloc.hpp"

RegisterAllocator::RegisterAllocator() {
    // Initialize register information
    register_info = {
        {Register::RAX, {"rax", true, true}},
        {Register::RBX, {"rbx", true, false}},
        {Register::RCX, {"rcx", true, true}},
        {Register::RDX, {"rdx", true, true}},
        {Register::RSI, {"rsi", true, true}},
        {Register::RDI, {"rdi", true, true}},
        {Register::R8, {"r8", true, true}},
        {Register::R9, {"r9", true, true}},
        {Register::R10, {"r10", true, true}},
        {Register::R11, {"r11", true, true}},
        {Register::R12, {"r12", true, false}},
        {Register::R13, {"r13", true, false}},
        {Register::R14, {"r14", true, false}},
        {Register::R15, {"r15", true, false}},
        {Register::EAX, {"eax", false, true}},
        {Register::EBX, {"ebx", false, false}},
        {Register::ECX, {"ecx", false, true}},
        {Register::EDX, {"edx", false, true}}
    };
    
    // Available general-purpose registers (excluding special ones)
    available_registers = {
        Register::RBX, Register::R12, Register::R13, Register::R14, Register::R15
    };
}

Register RegisterAllocator::allocate_register(const std::string& variable) {
    if (!variable.empty() && variable_registers.count(variable)) {
        return variable_registers[variable];
    }
    
    if (available_registers.empty()) {
        // Spill to memory - simplified for now
        return Register::RAX; // Fallback
    }
    
    Register reg = *available_registers.begin();
    available_registers.erase(reg);
    used_registers.insert(reg);
    
    if (!variable.empty()) {
        variable_registers[variable] = reg;
    }
    
    return reg;
}

void RegisterAllocator::free_register(Register reg) {
    if (used_registers.count(reg)) {
        used_registers.erase(reg);
        available_registers.insert(reg);
    }
}

std::string RegisterAllocator::get_register_name(Register reg) const {
    auto it = register_info.find(reg);
    return it != register_info.end() ? it->second.name : "unknown";
}

std::vector<Register> RegisterAllocator::get_caller_saved_used() const {
    std::vector<Register> result;
    for (Register reg : used_registers) {
        if (register_info.at(reg).caller_saved) {
            result.push_back(reg);
        }
    }
    return result;
}

std::vector<Register> RegisterAllocator::get_callee_saved_used() const {
    std::vector<Register> result;
    for (Register reg : used_registers) {
        if (!register_info.at(reg).caller_saved) {
            result.push_back(reg);
        }
    }
    return result;
}
