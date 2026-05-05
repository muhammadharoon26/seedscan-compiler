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
#include <sstream>

#include "seedscan/ast.hpp"
#include "seedscan/ast_printer.hpp"
#include "seedscan/semantic_analyzer.hpp"
#include "seedscan/ir_generator.hpp"
#include "seedscan/error_handler.hpp"

/* ── Flex/Bison interface ──────────────────────────────────────────────── */
extern FILE* yyin;          // Flex reads from this file pointer
extern int   yylex();
extern int   yyparse();     // Bison-generated parser entry point
extern int   yylineno;      // Current line number from Flex
extern int   yycolumn;
extern char* yytext;
extern void  yyrestart(FILE*);
struct YYLTYPE { int first_line, first_column, last_line, last_column; };
extern YYLTYPE yylloc;

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

    // ── PHASE 1: Lexical Analysis (Dump Tokens) ───────────────────────
    std::stringstream lexStream;
    if (printTokens || !outputFile.empty()) {
        FILE* fp1 = fopen(inputFile.c_str(), "r");
        if (!fp1) {
            std::cerr << "Error: Cannot open file '" << inputFile << "'\n";
            return 1;
        }
        yyin = fp1;
        yyrestart(fp1);
        yylineno = 1;
        yycolumn = 1;
        
        lexStream << "Line\tCol\tLexeme\n";
        lexStream << "--------------------------------------\n";
        while (yylex() != 0) {
            // Calculate starting column. yycolumn is already advanced by yyleng.
            int start_col = yycolumn - strlen(yytext);
            lexStream << yylineno << "\t" << start_col << "\t[" << yytext << "]\n";
        }
        fclose(fp1);

        if (printTokens) {
            std::cout << "\n--- Lexeme Tokens ---\n";
            std::cout << lexStream.str();
            return 0;
        }
    }

    // ── PHASE 2: Open source file for Parsing ──────────────────────────
    FILE* fp2 = fopen(inputFile.c_str(), "r");
    if (!fp2) {
        std::cerr << "Error: Cannot open file '" << inputFile << "'\n";
        return 1;
    }
    yyin = fp2;
    yyrestart(fp2);
    yylineno = 1;
    yycolumn = 1;

    std::cout << "=== SeedScan Mini-C++ Compiler ===\n";
    std::cout << "Compiling: " << inputFile << "\n\n";

    // ── PHASE 1-2: Lexing & Parsing (Flex + Bison) ────────────────────
    std::cout << "[Phase 1-2] Lexing & Parsing...\n";
    int parseResult = yyparse();
    fclose(fp2);

    if (parseResult != 0 || g_program == nullptr) {
        std::cerr << "Compilation failed: Parse errors detected.\n";
        return 2;
    }
    std::cout << "[Phase 1-2] Parsing complete. AST constructed.\n";

    // ── Optional: Print AST ───────────────────────────────────────────
    std::stringstream astStream;
    seedscan::ASTPrinter printer(astStream);
    g_program->accept(printer);

    if (printAST) {
        std::cout << "\n--- Abstract Syntax Tree ---\n";
        std::cout << astStream.str();
        delete g_program;
        return 0;
    }

    // ── PHASE 3: Semantic Analysis ────────────────────────────────────
    std::cout << "[Phase 3]   Semantic analysis...\n";
    seedscan::ErrorHandler errorHandler;
    seedscan::SemanticAnalyzer semanticAnalyzer(errorHandler);
    semanticAnalyzer.analyze(*g_program);

    std::stringstream semStream;
    if (errorHandler.hasErrors()) {
        std::cerr << "\n";
        errorHandler.printAll(std::cerr);
        errorHandler.printAll(semStream);
        std::cerr << "Compilation failed: Semantic errors detected.\n";
    } else {
        semStream << "Status: Passed\n";
        semStream << "Warnings: " << errorHandler.warningCount() << "\n";
        if (errorHandler.warningCount() > 0) {
            errorHandler.printAll(semStream);
        }
    }
    
    if (errorHandler.hasErrors()) {
        if (!outputFile.empty()) {
            std::ofstream ofs(outputFile);
            ofs << "============================================================\n";
            ofs << "Phase 1-2: Abstract Syntax Tree (AST)\n";
            ofs << "============================================================\n";
            ofs << astStream.str() << "\n";
            ofs << "============================================================\n";
            ofs << "Phase 3: Semantic Analysis (FAILED)\n";
            ofs << "============================================================\n";
            ofs << semStream.str();
            ofs.close();
        }
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

    // ── Output Full Report ────────────────────────────────────────────
    if (!outputFile.empty()) {
        std::ofstream ofs(outputFile);
        if (!ofs.is_open()) {
            std::cerr << "Error: Cannot write to '" << outputFile << "'\n";
            delete g_program;
            return 1;
        }
        ofs << "; SeedScan Compiler — Structured Compilation Report\n";
        ofs << "; Source: " << inputFile << "\n\n";

        ofs << "============================================================\n";
        ofs << "Phase 1: Lexeme Tokens Mapping\n";
        ofs << "============================================================\n";
        ofs << lexStream.str() << "\n";

        ofs << "============================================================\n";
        ofs << "Phase 2: Abstract Syntax Tree (AST)\n";
        ofs << "============================================================\n";
        ofs << astStream.str() << "\n";

        ofs << "============================================================\n";
        ofs << "Phase 3: Semantic Analysis\n";
        ofs << "============================================================\n";
        ofs << semStream.str() << "\n";

        ofs << "============================================================\n";
        ofs << "Phase 4: Three-Address Code (TAC)\n";
        ofs << "============================================================\n";
        ofs << tacOutput;
        ofs.close();
        std::cout << "\nStructured output written to: " << outputFile << "\n";
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
