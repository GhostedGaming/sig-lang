#pragma once
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

// Register management
enum class Register {
    RAX, RBX, RCX, RDX, RSI, RDI, R8, R9, R10, R11, R12, R13, R14, R15,
    EAX, EBX, ECX, EDX, ESI, EDI, R8D, R9D, R10D, R11D, R12D, R13D, R14D, R15D
};

struct RegisterInfo {
    std::string name;
    bool is_64bit;
    bool caller_saved;
    
    RegisterInfo(const std::string& n, bool is64, bool caller) 
        : name(n), is_64bit(is64), caller_saved(caller) {}
};

class RegisterAllocator {
private:
    std::unordered_map<Register, RegisterInfo> register_info;
    std::unordered_set<Register> available_registers;
    std::unordered_map<std::string, Register> variable_registers;
    std::unordered_set<Register> used_registers;
    
public:
    RegisterAllocator();
    
    Register allocate_register(const std::string& variable = "");
    void free_register(Register reg);
    std::string get_register_name(Register reg) const;
    std::vector<Register> get_caller_saved_used() const;
    std::vector<Register> get_callee_saved_used() const;
};
