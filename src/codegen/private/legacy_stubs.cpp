#include "../legacy/register_alloc.hpp"
#include "../legacy/optimization.hpp"
#include "../legacy/context.hpp"
#include "../legacy/templates.hpp"
#include <iostream>

// Stub implementations for legacy backend to allow compilation
RegisterAllocator::RegisterAllocator() {}
std::string RegisterAllocator::get_register_name(Register reg) const { return "stub"; }
std::vector<Register> RegisterAllocator::get_callee_saved_used() const { return {}; }

OptimizationManager::OptimizationManager() {}
void OptimizationManager::run_optimizations(std::vector<RTLInsn>& instructions, int opt_level) {}

void CodeGenContext::emit_data(const std::string& label, const std::string& data) {}
std::string CodeGenContext::new_label(const std::string& prefix) { return prefix + "_stub"; }

TemplateManager::TemplateManager() {}
const InstructionTemplate* TemplateManager::select_template(const RTLInsn& insn) { 
    static InstructionTemplate stub("", 
                                   [](const RTLInsn&) { return true; },
                                   [](const RTLInsn&, CodeGenContext&) { return std::string(""); },
                                   0, "");
    return &stub;
}
