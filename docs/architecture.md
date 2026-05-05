# SeedScan Compiler — Architecture Document

## Overview
SeedScan is a Mini-C++ compiler that translates a subset of C++ into Three-Address Code (TAC) intermediate representation. It is designed as a modular, four-phase pipeline, capped off with a lightweight virtual machine.

## Compilation Pipeline

```
┌─────────────┐    ┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│  Source Code │───▶│    Lexer     │───▶│   Parser    │───▶│  Semantic   │
│  (.mcpp)     │    │  (Flex)     │    │  (Bison)    │    │  Analyzer   │
│              │    │  scanner.l  │    │  parser.y   │    │             │
└─────────────┘    └──────┬──────┘    └──────┬──────┘    └──────┬──────┘
                          │                  │                  │
                     Token Stream          AST            Verified AST
                                                                │
                                                         ┌──────▼──────┐
                                                         │     IR      │
                                                         │  Generator  │
                                                         └──────┬──────┘
                                                                │
                                                         ┌──────▼──────┐
                                                         │   MiniVM    │
                                                         │ (Execution) │
                                                         └──────┬──────┘
                                                                │
                                                           Program Output
                                                           (Terminal)
```

## Phase Details

### Phase 1: Lexical Analysis (scanner.l)
- **Tool**: Flex
- **Input**: Raw source code characters
- **Output**: Stream of typed tokens
- **Key Features**: Line/column tracking, comment skipping, escape sequences
- **Files**: `src/scanner.l`

### Phase 2: Syntax Analysis (parser.y)
- **Tool**: Bison
- **Input**: Token stream from lexer
- **Output**: Abstract Syntax Tree (AST)
- **Key Features**: Operator precedence, dangling-else resolution
- **Files**: `src/parser.y`, `include/seedscan/ast.hpp`

### Phase 3: Semantic Analysis
- **Input**: AST from parser
- **Output**: Verified AST
- **Key Features**: Type checking, scoped symbol table, class member access
- **Files**: `src/semantic_analyzer.cpp`, `src/symbol_table.cpp`

### Phase 4: IR Generation
- **Input**: Verified AST
- **Output**: Three-Address Code (TAC)
- **Key Features**: Temp variables, labels, control flow patterns
- **Files**: `src/ir_generator.cpp`, `include/seedscan/tac.hpp`

### Phase 5: Execution (MiniVM)
- **Input**: TAC Output
- **Output**: Runtime program execution
- **Key Features**: Instruction pointer, call stack, native I/O interception
- **Files**: `src/minivm.cpp`

## Structured Compilation Report
When compiling with the `-o` flag, the compiler generates a multi-phase report containing:
1. **Phase 1: Lexeme Mapping**: Line/Col details for every token.
2. **Phase 2: AST**: Full visual tree of the program structure.
3. **Phase 3: Semantic Status**: Validation result and warning count.
4. **Phase 4: TAC**: The resulting Three-Address Code.

## Design Patterns

### Visitor Pattern
All AST traversals (printing, semantic analysis, IR generation) use the Visitor pattern. This separates tree structure from operations, allowing new passes without modifying nodes.

### Scope Stack (Symbol Table)
The symbol table uses a stack of hash maps. `enterScope()` pushes a new map; `exitScope()` pops it. Lookup searches from innermost to outermost (lexical scoping).

## Build System
- **CMake 3.16+** with C++17
- **MinGW GCC** (g++) on Windows
- **Flex/Bison** via winflexbison
- **Google Test** via CMake FetchContent
