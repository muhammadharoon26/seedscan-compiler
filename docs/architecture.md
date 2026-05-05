# SeedScan Compiler — Architecture Document

## Overview
SeedScan is a Mini-C++ compiler that translates a subset of C++ into Three-Address Code (TAC) intermediate representation. It is designed as a modular, four-phase pipeline.

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
                                                          TAC Output
                                                          (.tac file)
```

## Phase Details

### Phase 1: Lexical Analysis (scanner.l)
- **Tool**: Flex (winflexbison on Windows)
- **Input**: Raw source code characters
- **Output**: Stream of typed tokens
- **Key Features**: Line/column tracking, comment skipping, escape sequences
- **Files**: `src/scanner.l`, `include/seedscan/token.hpp`

### Phase 2: Syntax Analysis (parser.y)
- **Tool**: Bison
- **Input**: Token stream from lexer
- **Output**: Abstract Syntax Tree (AST)
- **Key Features**: Operator precedence, dangling-else resolution, error recovery
- **Files**: `src/parser.y`, `include/seedscan/ast.hpp`, `include/seedscan/ast_visitor.hpp`

### Phase 3: Semantic Analysis
- **Input**: AST from parser
- **Output**: Verified AST (same structure, validated semantics)
- **Key Features**: Type checking with implicit promotion, scoped symbol table, class member access control
- **Files**: `src/semantic_analyzer.cpp`, `src/symbol_table.cpp`, `include/seedscan/symbol_table.hpp`

### Phase 4: IR Generation
- **Input**: Verified AST
- **Output**: Three-Address Code (data structure + text file)
- **Key Features**: Temp variables, labels, control flow patterns
- **Files**: `src/ir_generator.cpp`, `include/seedscan/tac.hpp`

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
