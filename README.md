# SeedScan Compiler

A Mini-C++ compiler for an 8th-semester Compiler Construction academic project.

## Features
- **Lexer** (Flex): Tokenizes Mini-C++ source into a stream of typed tokens
- **Parser** (Bison): Builds an Abstract Syntax Tree (AST) from tokens
- **Semantic Analyzer**: Type checking and scope resolution via symbol table
- **IR Generator**: Produces Three-Address Code (TAC) intermediate representation

## Supported Language Features
- Data types: `int`, `float`, `char`, `bool`, `void`
- Arithmetic, comparison, logical, assignment operators
- Increment/decrement (`++`, `--`)
- Control flow: `if`/`else`, `while`, `for`
- Functions with typed parameters and return values
- Arrays (fixed-size)
- Basic classes (members, methods, public/private, constructor)
- I/O: `cout <<`, `cin >>`
- Comments: `//` and `/* */`

## Prerequisites (Windows)
- MinGW GCC (g++) — C++17 support
- CMake 3.16+
- winflexbison3 (`choco install winflexbison3`)

## Build
```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

## Usage
```powershell
.\build\minicc.exe input.mcpp -o output.tac
.\build\minicc.exe input.mcpp --ast       # Print AST only
.\build\minicc.exe input.mcpp --tokens    # Print tokens only
```

## Run Tests
```powershell
cd build
ctest --output-on-failure
```

## Author
muhammadharoon26
