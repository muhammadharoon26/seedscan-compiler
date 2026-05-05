########################################################################
# SeedScan Compiler — Test Configuration
# -----------------------------------------------------------------------
# Fetches Google Test and registers all test suites.
########################################################################

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
# Prevent overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# ── Compiler library (all sources except main.cpp) for test linking ────
add_library(seedscan_lib STATIC
    ${CMAKE_SOURCE_DIR}/src/ast.cpp
    ${CMAKE_SOURCE_DIR}/src/ast_printer.cpp
    ${CMAKE_SOURCE_DIR}/src/symbol_table.cpp
    ${CMAKE_SOURCE_DIR}/src/semantic_analyzer.cpp
    ${CMAKE_SOURCE_DIR}/src/ir_generator.cpp
    ${CMAKE_SOURCE_DIR}/src/error_handler.cpp
    ${CMAKE_BINARY_DIR}/src/scanner.cpp
    ${CMAKE_BINARY_DIR}/src/parser.cpp
)
target_include_directories(seedscan_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/src
)

# ── Test executables ───────────────────────────────────────────────────
# Lexer tests
add_executable(lexer_test lexer_test.cpp)
target_link_libraries(lexer_test PRIVATE seedscan_lib GTest::gtest_main)
gtest_discover_tests(lexer_test)

# Parser tests
add_executable(parser_test parser_test.cpp)
target_link_libraries(parser_test PRIVATE seedscan_lib GTest::gtest_main)
gtest_discover_tests(parser_test)

# Semantic tests
add_executable(semantic_test semantic_test.cpp)
target_link_libraries(semantic_test PRIVATE seedscan_lib GTest::gtest_main)
gtest_discover_tests(semantic_test)

# IR tests
add_executable(ir_test ir_test.cpp)
target_link_libraries(ir_test PRIVATE seedscan_lib GTest::gtest_main)
gtest_discover_tests(ir_test)
