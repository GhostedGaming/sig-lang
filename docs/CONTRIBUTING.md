# Contributing to Sig

Thank you for your interest in contributing to Sig! This document provides guidelines and information for contributors.

## Getting Started

### Development Setup
1. **Clone the repository**:
   ```bash
   git clone https://github.com/GhostedGaming/sig-language.git
   cd sig-language
   ```

2. **Install dependencies**:
   - LLVM 18+ with development headers
   - CMake 3.20+
   - C++23 compatible compiler

3. **Build and test**:
   ```bash
   make
   ./sig examples/hello_world.sg
   ```

## Code Organization

```
src/
├── lexer/          # Tokenization (lexer.hpp, lexer.cpp)
├── parser/         # Syntax analysis (parser.hpp, parser.cpp)  
├── ast/            # AST definitions (ast.hpp)
├── codegen/        # LLVM code generation
│   ├── public/     # Headers (codegen.hpp, llvm_codegen.hpp)
│   ├── private/    # Implementation (*.cpp)
│   └── utils/      # Utilities
└── main.cpp        # Compiler driver
```

## Development Priorities

### High Priority
- **Control Flow**: Implement if/else, while loops, for loops
- **Operators**: Arithmetic, comparison, logical operators
- **Type System**: Strong typing, type checking, inference
- **Error Handling**: Better error messages and recovery

### Medium Priority  
- **Standard Library**: Core functions, data structures
- **Module System**: Import/export, package management
- **Optimization**: LLVM passes, custom optimizations
- **Tooling**: Language server, debugger integration

### Low Priority
- **Advanced Features**: Generics, macros, metaprogramming
- **Platform Support**: Windows native, embedded targets
- **IDE Integration**: Syntax highlighting, autocomplete

## Code Style

- **C++23** standard features preferred
- **LLVM coding standards** for LLVM-related code
- **Clear naming**: Functions, variables, classes should be self-documenting
- **Comments**: Explain why, not what
- **Error handling**: Always check LLVM error returns

## Testing

- Add test cases in `examples/` for new features
- Test with all compilation modes: `--jit`, `--ir`, `--legacy`
- Ensure LLVM IR is valid and well-formed

## Pull Request Process

1. Create a feature branch from `main`
2. Make your changes with clear, focused commits
3. Add examples demonstrating new features
4. Test thoroughly across compilation modes
5. Update documentation as needed
6. Submit PR with clear description

## Questions?

- Open an issue for bugs or feature requests
- Join discussions in existing issues
- Check the README for basic information

Happy coding! 🚀
