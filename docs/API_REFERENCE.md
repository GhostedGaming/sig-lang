# Sig Language API Reference

## Table of Contents
- [Compiler Options](#compiler-options)
- [Built-in Functions](#built-in-functions)
- [Data Types](#data-types)
- [Operators](#operators)
- [Keywords](#keywords)
- [Syntax Grammar](#syntax-grammar)
- [Error Codes](#error-codes)

## Compiler Options

The Sig compiler supports various command-line options:

### Basic Usage
```bash
sig [OPTIONS] <input_file>
```

### Options

| Option | Description | Example |
|--------|-------------|---------|
| `-o <name>` | Specify output file name | `sig program.sg -o myapp` |
| `--jit` | Execute with LLVM JIT (no file output) | `sig program.sg --jit` |
| `--ir` | Output LLVM IR instead of executable | `sig program.sg --ir` |
| `--legacy` | Use legacy x86-64 backend | `sig program.sg --legacy` |
| `--32bit` | Target 32-bit architecture | `sig program.sg --32bit` |
| `--no-std` | Disable standard library (for OS/kernel development) | `sig kernel.sg --no-std` |
| `--object` | Create object file only | `sig program.sg --object` |
| `--help` | Show help message | `sig --help` |
| `--version` | Show version information | `sig --version` |

### Examples
```bash
# Compile to executable (default)
sig hello.sg

# Custom output name
sig hello.sg -o hello_world

# JIT execution for testing
sig hello.sg --jit

# View generated LLVM IR
sig hello.sg --ir

# Create object file for linking
sig hello.sg --object -o hello.o

# Compile kernel/OS code without standard library
sig bootloader.sg --no-std -o bootloader.bin
```

## No-Std Mode

The `--no-std` option disables the standard library for operating system and kernel development. 

### What's Disabled in No-Std Mode
- `print()` and `println()` functions
- All standard library functions (input, len, abs, sqrt, max, min)
- C runtime library dependencies

### When to Use No-Std Mode
- Kernel development
- Bootloader development  
- Embedded systems programming
- Bare metal programming
- Operating system development

### Example: Kernel Development
```sig
// kernel.sg - Example kernel code
fn kernel_main() {
    // Direct hardware access only
    let vga_buffer: *u16 = 0xB8000 as *u16;
    // Custom I/O implementation required
}
```

Compile with:
```bash
sig kernel.sg --no-std -o kernel.bin
```

## Built-in Functions

### I/O Functions

#### `print(value)`
Outputs a value to stdout without a newline.

**Parameters:**
- `value`: Any printable type (string, integer, etc.)

**Example:**
```sig
print("Hello");
print(42);
```

#### `println(value)`
Outputs a value to stdout with a trailing newline.

**Parameters:**
- `value`: Any printable type (string, integer, etc.)

**Example:**
```sig
println("Hello, World!");
println(123);
```

### Type Conversion Functions

#### `as` (Type Casting)
Converts a value from one type to another.

**Syntax:**
```sig
value as target_type
```

**Example:**
```sig
let addr: u32 = 0xb8000;
let ptr: *u16 = addr as *u16;
```

## Data Types

### Integer Types

| Type | Size | Range | Description |
|------|------|-------|-------------|
| `u8` | 8 bits | 0 to 255 | Unsigned byte |
| `u16` | 16 bits | 0 to 65,535 | Unsigned word |
| `u32` | 32 bits | 0 to 4,294,967,295 | Unsigned double word |
| `u64` | 64 bits | 0 to 18,446,744,073,709,551,615 | Unsigned quad word |
| `i8` | 8 bits | -128 to 127 | Signed byte |
| `i16` | 16 bits | -32,768 to 32,767 | Signed word |
| `i32` | 32 bits | -2,147,483,648 to 2,147,483,647 | Signed double word |
| `i64` | 64 bits | -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807 | Signed quad word |

### Floating Point Types

| Type | Size | Precision | Description |
|------|------|-----------|-------------|
| `f32` | 32 bits | ~7 decimal digits | Single precision float |
| `f64` | 64 bits | ~15 decimal digits | Double precision float |

### Other Types

| Type | Description | Example |
|------|-------------|---------|
| `bool` | Boolean value | `true`, `false` |
| `string` | UTF-8 string | `"Hello, World!"` |
| `*T` | Pointer to type T | `*u32`, `*i8` |

### Type Literals

#### Integer Literals
```sig
42          // Decimal
0x2A        // Hexadecimal
0b101010    // Binary (planned)
0o52        // Octal (planned)
```

#### Float Literals
```sig
3.14        // Standard notation
2.5e10      // Scientific notation (planned)
```

#### String Literals
```sig
"Hello"     // Standard string
"Line 1\nLine 2"  // With escape sequences (planned)
```

#### Boolean Literals
```sig
true
false
```

## Operators

### Arithmetic Operators

| Operator | Description | Example | Precedence |
|----------|-------------|---------|------------|
| `+` | Addition | `a + b` | 6 |
| `-` | Subtraction | `a - b` | 6 |
| `*` | Multiplication | `a * b` | 7 |
| `/` | Division | `a / b` | 7 |
| `%` | Modulo | `a % b` | 7 |

### Comparison Operators

| Operator | Description | Example | Precedence |
|----------|-------------|---------|------------|
| `==` | Equal to | `a == b` | 4 |
| `!=` | Not equal to | `a != b` | 4 |
| `<` | Less than | `a < b` | 5 |
| `<=` | Less than or equal | `a <= b` | 5 |
| `>` | Greater than | `a > b` | 5 |
| `>=` | Greater than or equal | `a >= b` | 5 |

### Logical Operators

| Operator | Description | Example | Precedence |
|----------|-------------|---------|------------|
| `&&` | Logical AND | `a && b` | 2 |
| `\|\|` | Logical OR | `a \|\| b` | 1 |
| `!` | Logical NOT | `!a` | 8 |

### Bitwise Operators

| Operator | Description | Example | Precedence |
|----------|-------------|---------|------------|
| `&` | Bitwise AND | `a & b` | 3 |
| `\|` | Bitwise OR | `a \| b` | 3 |
| `^` | Bitwise XOR | `a ^ b` | 3 |
| `<<` | Left shift | `a << 2` | 6 |
| `>>` | Right shift | `a >> 1` | 6 |

### Assignment Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `=` | Assignment | `a = b` |

## Keywords

### Control Flow Keywords
- `if` - Conditional execution
- `elif` - Else if condition
- `else` - Alternative condition
- `while` - While loop
- `for` - For loop
- `return` - Return from function

### Declaration Keywords
- `let` - Variable declaration
- `fn` - Function declaration
- `struct` - Structure declaration
- `import` - Module import

### Type Keywords
- `u8`, `u16`, `u32`, `u64` - Unsigned integers
- `i8`, `i16`, `i32`, `i64` - Signed integers
- `f32`, `f64` - Floating point numbers
- `bool` - Boolean type
- `as` - Type casting

### Literal Keywords
- `true` - Boolean true
- `false` - Boolean false

### Reserved Keywords (Future Use)
- `const` - Constant declaration
- `mut` - Mutable declaration
- `unsafe` - Unsafe code block
- `asm` - Inline assembly
- `match` - Pattern matching
- `enum` - Enumeration
- `trait` - Trait definition
- `impl` - Implementation

## Syntax Grammar

### Program Structure
```bnf
program ::= statement*

statement ::= variable_declaration
           | function_declaration
           | struct_declaration
           | import_statement
           | expression_statement
           | return_statement
           | control_flow_statement

expression_statement ::= expression ";"
```

### Variable Declaration
```bnf
variable_declaration ::= "let" identifier (":" type)? "=" expression ";"
```

### Function Declaration
```bnf
function_declaration ::= "fn" identifier "(" parameter_list? ")" ("->" type)? block

parameter_list ::= parameter ("," parameter)*
parameter ::= identifier ":" type
```

### Struct Declaration
```bnf
struct_declaration ::= "struct" identifier "{" field_list "}"

field_list ::= field ("," field)*
field ::= identifier ":" type
```

### Control Flow
```bnf
if_statement ::= "if" "(" expression ")" block ("elif" "(" expression ")" block)* ("else" block)?

while_statement ::= "while" "(" expression ")" block

for_statement ::= "for" "(" identifier "," expression "," expression ")" block
```

### Expressions
```bnf
expression ::= logical_or_expression

logical_or_expression ::= logical_and_expression ("||" logical_and_expression)*
logical_and_expression ::= equality_expression ("&&" equality_expression)*
equality_expression ::= relational_expression (("==" | "!=") relational_expression)*
relational_expression ::= additive_expression (("<" | "<=" | ">" | ">=") additive_expression)*
additive_expression ::= multiplicative_expression (("+" | "-") multiplicative_expression)*
multiplicative_expression ::= unary_expression (("*" | "/" | "%") unary_expression)*
unary_expression ::= ("!" | "-")? primary_expression

primary_expression ::= literal
                    | identifier
                    | function_call
                    | "(" expression ")"
                    | cast_expression

cast_expression ::= expression "as" type
function_call ::= identifier "(" argument_list? ")"
argument_list ::= expression ("," expression)*
```

### Types
```bnf
type ::= primitive_type
      | pointer_type
      | struct_type

primitive_type ::= "u8" | "u16" | "u32" | "u64"
                | "i8" | "i16" | "i32" | "i64"
                | "f32" | "f64"
                | "bool"

pointer_type ::= "*" type
struct_type ::= identifier
```

### Literals
```bnf
literal ::= integer_literal
         | float_literal
         | string_literal
         | boolean_literal

integer_literal ::= decimal_literal | hex_literal
decimal_literal ::= [0-9]+
hex_literal ::= "0x" [0-9a-fA-F]+

float_literal ::= [0-9]+ "." [0-9]+

string_literal ::= "\"" [^"]* "\""

boolean_literal ::= "true" | "false"
```

## Error Codes

### Compilation Errors

| Code | Message | Description |
|------|---------|-------------|
| E001 | Syntax error | Invalid syntax in source code |
| E002 | Undefined variable | Variable used before declaration |
| E003 | Type mismatch | Incompatible types in operation |
| E004 | Undefined function | Function called but not defined |
| E005 | Invalid cast | Cannot cast between specified types |
| E006 | Missing return | Function doesn't return required value |

### Runtime Errors

| Code | Message | Description |
|------|---------|-------------|
| R001 | Division by zero | Attempted division by zero |
| R002 | Stack overflow | Function call stack exceeded |
| R003 | Segmentation fault | Invalid memory access |

### Compiler Internal Errors

| Code | Message | Description |
|------|---------|-------------|
| I001 | LLVM error | Internal LLVM compilation error |
| I002 | Linker error | Error during linking phase |
| I003 | File I/O error | Cannot read or write file |

## Version Information

Current API version: 0.2.0-alpha

### Version History
- 0.1.0: Initial release with basic features
- 0.2.0: LLVM integration, improved type system

### Compatibility
- LLVM 18+ required
- C++23 compatible compiler needed
- CMake 3.20+ for building

## See Also
- [Language Guide](LANGUAGE_GUIDE.md) - Complete language tutorial
- [Examples](../examples/) - Working code examples
- [README](../README.md) - Project overview and setup
