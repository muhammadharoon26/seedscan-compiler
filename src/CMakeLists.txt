########################################################################
# SeedScan Compiler — Source Build Configuration
# -----------------------------------------------------------------------
# Generates scanner (Flex) and parser (Bison), then compiles all
# compiler phase source files into the 'minicc' executable.
########################################################################

# ── Generate scanner from Flex specification ───────────────────────────
FLEX_TARGET(Scanner
    ${CMAKE_CURRENT_SOURCE_DIR}/scanner.l
    ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/scanner.hpp
)

# ── Generate parser from Bison grammar ─────────────────────────────────
BISON_TARGET(Parser
    ${CMAKE_CURRENT_SOURCE_DIR}/parser.y
    ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.hpp
)

# ── Link Flex output to Bison output ───────────────────────────────────
ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)

# ── Compiler executable ────────────────────────────────────────────────
add_executable(minicc
    main.cpp
    ast.cpp
    ast_printer.cpp
    symbol_table.cpp
    semantic_analyzer.cpp
    ir_generator.cpp
    error_handler.cpp
    ${FLEX_Scanner_OUTPUTS}
    ${BISON_Parser_OUTPUTS}
)

# ── Include generated headers (parser.hpp, scanner.hpp) ────────────────
target_include_directories(minicc PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/include
)
