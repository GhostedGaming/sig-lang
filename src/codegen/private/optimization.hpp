#pragma once
#include "rtl.hpp"
#include <vector>
#include <memory>
#include <string>

// Optimization passes
class OptimizationPass {
public:
    virtual ~OptimizationPass() = default;
    virtual bool run(std::vector<RTLInsn>& instructions) = 0;
    virtual std::string name() const = 0;
};

class ConstantFoldingPass : public OptimizationPass {
public:
    bool run(std::vector<RTLInsn>& instructions) override;
    std::string name() const override;
};

class DeadCodeEliminationPass : public OptimizationPass {
public:
    bool run(std::vector<RTLInsn>& instructions) override;
    std::string name() const override;
};

class PeepholeOptimizationPass : public OptimizationPass {
public:
    bool run(std::vector<RTLInsn>& instructions) override;
    std::string name() const override;
};

// Optimization manager
class OptimizationManager {
private:
    std::vector<std::unique_ptr<OptimizationPass>> passes;
    
public:
    OptimizationManager();
    void run_optimizations(std::vector<RTLInsn>& instructions, int max_iterations = 10);
};
