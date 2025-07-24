# Sig Language Examples

This directory contains example programs demonstrating Sig language features.

## Running Examples

All examples can be run with any compilation mode:

```bash
# Compile to executable (default)
sig examples/hello_world.sg
./hello_world

# Custom output name
sig examples/variables.sg -o myprogram
./myprogram

# JIT execution (fastest for development)
sig examples/functions.sg --jit

# View generated LLVM IR
sig examples/variables.sg --ir

# Use legacy x86-64 backend
sig examples/functions.sg --legacy
```

## Available Examples

### Basic Features

- **`hello_world.sg`** - Simple "Hello, World!" program
- **`variables.sg`** - Variable declaration and assignment
- **`functions.sg`** - Function definitions and calls

### Advanced Features

- **`comprehensive.sg`** - Full feature demonstration (includes unimplemented features)

## Example Structure

Each example is self-contained and demonstrates specific language features:

```sig
// Comments explain the feature being demonstrated
let variable = "example";  // Variable with string
print(variable);           // Print statement

fn my_function() {         // Function definition
    print("In function");
}
my_function();            // Function call
```

## Contributing Examples

When adding new language features, please:

1. Create a focused example in this directory
2. Add clear comments explaining the feature
3. Test with all compilation modes
4. Update this README with the new example

## Notes

- Examples use `.sg` file extension for Sig programs
- All examples should compile and run without errors
- Focus on demonstrating language features clearly
- Keep examples simple and educational
