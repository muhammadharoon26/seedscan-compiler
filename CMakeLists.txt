########################################################################
# SeedScan Compiler — Root CMake Configuration
# -----------------------------------------------------------------------
# Build system for the Mini-C++ compiler.
# Uses Flex + Bison for scanner/parser generation, MinGW GCC toolchain.
########################################################################
cmake_minimum_required(VERSION 3.16)
project(seedscan-compiler
    VERSION 1.0.0
    LANGUAGES C CXX
    DESCRIPTION "Mini-C++ Compiler — 8th Semester Academic Project"
)

# ── C++17 Standard ─────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Output directories ─────────────────────────────────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ── Global include path ────────────────────────────────────────────────
include_directories(${CMAKE_SOURCE_DIR}/include)

# ── Find Flex & Bison ──────────────────────────────────────────────────
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

# ── Subdirectories ─────────────────────────────────────────────────────
add_subdirectory(src)

# ── Testing (optional, enable with -DBUILD_TESTS=ON) ──────────────────
option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
