#include "optimization.hpp"
#include <unordered_set>
#include <algorithm>

bool ConstantFoldingPass::run(std::vector<RTLInsn>& instructions) {
    bool changed = false;
    
    for (auto& insn : instructions) {
        if (insn.op == RTLInsn::ADD && insn.operands.size() >= 3) {
            // Try to fold constant additions
            try {
                int val1 = std::stoi(insn.operands[1]);
                int val2 = std::stoi(insn.operands[2]);
                int result = val1 + val2;
                
                // Replace with MOV immediate
                insn.op = RTLInsn::MOV;
                insn.operands = {insn.operands[0], std::to_string(result)};
                changed = true;
            } catch (...) {
                // Not constants, skip
            }
        }
    }
    
    return changed;
}

std::string ConstantFoldingPass::name() const {
    return "ConstantFolding";
}

bool DeadCodeEliminationPass::run(std::vector<RTLInsn>& instructions) {
    bool changed = false;
    std::unordered_set<std::string> used_vars;
    
    // Mark used variables (backward pass)
    for (auto it = instructions.rbegin(); it != instructions.rend(); ++it) {
        for (size_t i = 1; i < it->operands.size(); ++i) {
            used_vars.insert(it->operands[i]);
        }
    }
    
    // Remove unused assignments
    instructions.erase(
        std::remove_if(instructions.begin(), instructions.end(),
            [&](const RTLInsn& insn) {
                if (insn.op == RTLInsn::MOV && !insn.operands.empty()) {
                    return used_vars.find(insn.operands[0]) == used_vars.end();
                }
                return false;
            }), 
        instructions.end());
    
    return changed;
}

std::string DeadCodeEliminationPass::name() const {
    return "DeadCodeElimination";
}

bool PeepholeOptimizationPass::run(std::vector<RTLInsn>& instructions) {
    bool changed = false;
    
    for (size_t i = 0; i < instructions.size() - 1; ++i) {
        auto& curr = instructions[i];
        auto& next = instructions[i + 1];
        
        // Pattern: mov reg, val; mov reg2, reg -> mov reg2, val
        if (curr.op == RTLInsn::MOV && next.op == RTLInsn::MOV &&
            curr.operands.size() >= 2 && next.operands.size() >= 2 &&
            curr.operands[0] == next.operands[1]) {
            
            next.operands[1] = curr.operands[1];
            instructions.erase(instructions.begin() + i);
            changed = true;
            --i; // Recheck this position
        }
        
        // Pattern: xor reg, reg (zero register)
        if (curr.op == RTLInsn::XOR && curr.operands.size() >= 2 &&
            curr.operands[0] == curr.operands[1]) {
            // This is already optimal for zeroing
            continue;
        }
    }
    
    return changed;
}

std::string PeepholeOptimizationPass::name() const {
    return "PeepholeOptimization";
}

OptimizationManager::OptimizationManager() {
    passes.push_back(std::make_unique<ConstantFoldingPass>());
    passes.push_back(std::make_unique<DeadCodeEliminationPass>());
    passes.push_back(std::make_unique<PeepholeOptimizationPass>());
}

void OptimizationManager::run_optimizations(std::vector<RTLInsn>& instructions, int max_iterations) {
    bool changed;
    int iteration = 0;
    
    do {
        changed = false;
        for (auto& pass : passes) {
            if (pass->run(instructions)) {
                changed = true;
            }
        }
        ++iteration;
    } while (changed && iteration < max_iterations);
}
