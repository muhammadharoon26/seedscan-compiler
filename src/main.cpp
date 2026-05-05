/***************************************************************************
 * main.cpp — CLI Driver for SeedScan Mini-C++ Compiler
 * -------------------------------------------------------------------------
 * Orchestrates the compilation pipeline:
 *   Source File → Lexer → Parser → AST → Semantic → IR → Output
 *
 * Usage:
 *   minicc <input.mcpp> [-o output.tac] [--ast] [--tokens]
 *
 * Exit Codes:
 *   0 — Success
 *   1 — File I/O error
 *   2 — Parse error
 *   3 — Semantic error
 ***************************************************************************/
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "seedscan/ast.hpp"
#include "seedscan/ast_printer.hpp"
#include "seedscan/semantic_analyzer.hpp"
#include "seedscan/ir_generator.hpp"
#include "seedscan/error_handler.hpp"

/* ── Flex/Bison interface ──────────────────────────────────────────────── */
extern FILE* yyin;          // Flex reads from this file pointer
extern int   yyparse();     // Bison-generated parser entry point
extern int   yylineno;      // Current line number from Flex

/* Global AST root — set by the parser */
extern seedscan::Program* g_program;

/* ── Print usage help ──────────────────────────────────────────────────── */
static void printUsage(const char* progName) {
    std::cerr << "SeedScan Mini-C++ Compiler v1.0\n"
              << "Usage: " << progName << " <input.mcpp> [options]\n\n"
              << "Options:\n"
              << "  -o <file>    Write TAC output to file (default: stdout)\n"
              << "  --ast        Print the Abstract Syntax Tree and exit\n"
              << "  --tokens     Print the token stream and exit\n"
              << "  --help       Show this help message\n";
}

/* ══════════════════════════════════════════════════════════════════════════
 * MAIN — Compilation Pipeline
 * ════════════════════════════════════════════════════════════════════════*/
int main(int argc, char* argv[]) {
    // ── Parse CLI arguments ───────────────────────────────────────────
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string inputFile;
    std::string outputFile;
    bool printAST    = false;
    bool printTokens = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printUsage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--ast") == 0) {
            printAST = true;
        } else if (strcmp(argv[i], "--tokens") == 0) {
            printTokens = true;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            outputFile = argv[++i];
        } else if (inputFile.empty()) {
            inputFile = argv[i];
        } else {
            std::cerr << "Error: Unknown argument '" << argv[i] << "'\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    if (inputFile.empty()) {
        std::cerr << "Error: No input file specified.\n";
        printUsage(argv[0]);
        return 1;
    }

    // ── PHASE 1: Open source file for Flex ────────────────────────────
    FILE* fp = fopen(inputFile.c_str(), "r");
    if (!fp) {
        std::cerr << "Error: Cannot open file '" << inputFile << "'\n";
        return 1;
    }
    yyin = fp;

    std::cout << "=== SeedScan Mini-C++ Compiler ===\n";
    std::cout << "Compiling: " << inputFile << "\n\n";

    // ── PHASE 2: Lexical + Syntax Analysis (Flex + Bison) ─────────────
    std::cout << "[Phase 1-2] Lexing & Parsing...\n";
    int parseResult = yyparse();
    fclose(fp);

    if (parseResult != 0 || g_program == nullptr) {
        std::cerr << "Compilation failed: Parse errors detected.\n";
        return 2;
    }
    std::cout << "[Phase 1-2] Parsing complete. AST constructed.\n";

    // ── Optional: Print AST ───────────────────────────────────────────
    if (printAST) {
        std::cout << "\n--- Abstract Syntax Tree ---\n";
        seedscan::ASTPrinter printer(std::cout);
        g_program->accept(printer);
        delete g_program;
        return 0;
    }

    // ── PHASE 3: Semantic Analysis ────────────────────────────────────
    std::cout << "[Phase 3]   Semantic analysis...\n";
    seedscan::ErrorHandler errorHandler;
    seedscan::SemanticAnalyzer semanticAnalyzer(errorHandler);
    semanticAnalyzer.analyze(*g_program);

    if (errorHandler.hasErrors()) {
        std::cerr << "\n";
        errorHandler.printAll(std::cerr);
        std::cerr << "Compilation failed: Semantic errors detected.\n";
        delete g_program;
        return 3;
    }
    std::cout << "[Phase 3]   Semantic analysis passed. "
              << errorHandler.warningCount() << " warning(s).\n";

    // ── PHASE 4: IR Generation ────────────────────────────────────────
    std::cout << "[Phase 4]   Generating Three-Address Code...\n";
    seedscan::IRGenerator irGen;
    irGen.generate(*g_program);

    std::string tacOutput = irGen.getProgram().toString();
    std::cout << "[Phase 4]   IR generation complete. "
              << irGen.getProgram().instructions().size() << " instructions.\n";

    // ── Output TAC ────────────────────────────────────────────────────
    if (!outputFile.empty()) {
        std::ofstream ofs(outputFile);
        if (!ofs.is_open()) {
            std::cerr << "Error: Cannot write to '" << outputFile << "'\n";
            delete g_program;
            return 1;
        }
        ofs << "; SeedScan Compiler — Three-Address Code Output\n";
        ofs << "; Source: " << inputFile << "\n\n";
        ofs << tacOutput;
        ofs.close();
        std::cout << "\nTAC written to: " << outputFile << "\n";
    } else {
        std::cout << "\n--- Three-Address Code ---\n";
        std::cout << tacOutput;
    }

    if (errorHandler.warningCount() > 0) {
        std::cout << "\n";
        errorHandler.printAll(std::cout);
    }

    std::cout << "\n=== Compilation Successful ===\n";

    delete g_program;
    return 0;
}
