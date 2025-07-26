#include "../public/codegen.hpp"
#include <llvm/Support/raw_ostream.h>
#include <iostream>

using namespace llvm;

void CodeGen::execute() {
    auto ProcessSymsGenerator = orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
        jit->getDataLayout().getGlobalPrefix());
    if (!ProcessSymsGenerator) {
        std::cerr << "Failed to create process symbols generator: " 
                  << toString(ProcessSymsGenerator.takeError()) << std::endl;
        return;
    }
    jit->getMainJITDylib().addGenerator(std::move(*ProcessSymsGenerator));
    
    auto TSM = orc::ThreadSafeModule(std::move(module), std::move(context));
    
    if (auto Err = jit->addIRModule(std::move(TSM))) {
        std::cerr << "Failed to add module: " << toString(std::move(Err)) << std::endl;
        return;
    }
    
    auto MainSym = jit->lookup("main");
    if (!MainSym) {
        std::cerr << "Failed to find main function: " << toString(MainSym.takeError()) << std::endl;
        return;
    }
    
    auto MainFunc = (int(*)())MainSym->getValue();
    int result = MainFunc();
    std::cout << "Program exited with code: " << result << std::endl;
}

void CodeGen::dump_ir() {
    if (module) {
        module->print(outs(), nullptr);
    } else {
        std::cerr << "Error: Module has been moved or is null" << std::endl;
    }
}
