/***************************************************************************
 * ir_test.cpp — Unit Tests for the TAC IR Generator
 * -------------------------------------------------------------------------
 * Tests that the IR generator produces correct three-address code
 * for arithmetic, control flow, functions, and I/O.
 ***************************************************************************/
#include <gtest/gtest.h>
#include <cstdio>
#include <string>
#include "seedscan/ast.hpp"
#include "seedscan/semantic_analyzer.hpp"
#include "seedscan/ir_generator.hpp"
#include "seedscan/error_handler.hpp"

extern FILE* yyin;
extern int   yyparse();
extern int   yylineno;
extern int   yycolumn;
extern seedscan::Program* g_program;

class IRTest : public ::testing::Test {
protected:
    FILE* tempFile_ = nullptr;

    std::string GenerateIR(const std::string& code) {
        tempFile_ = tmpfile();
        if (!tempFile_) return "";
        fputs(code.c_str(), tempFile_);
        rewind(tempFile_);
        yyin = tempFile_;
        yylineno = 1; yycolumn = 1;
        g_program = nullptr;
        if (yyparse() != 0 || !g_program) return "";

        seedscan::ErrorHandler errors;
        seedscan::SemanticAnalyzer semantic(errors);
        semantic.analyze(*g_program);

        seedscan::IRGenerator irGen;
        irGen.generate(*g_program);
        return irGen.getProgram().toString();
    }

    void TearDown() override {
        if (tempFile_) fclose(tempFile_);
        if (g_program) { delete g_program; g_program = nullptr; }
    }
};

TEST_F(IRTest, GeneratesAssignment) {
    std::string tac = GenerateIR("int main() { int x = 42; return 0; }");
    EXPECT_FALSE(tac.empty());
    EXPECT_NE(tac.find("x = 42"), std::string::npos);
}

TEST_F(IRTest, GeneratesArithmeticTAC) {
    std::string tac = GenerateIR("int main() { int a = 1; int b = 2; int c = a + b; return 0; }");
    EXPECT_FALSE(tac.empty());
    // Should contain: t0 = a + b, c = t0
    EXPECT_NE(tac.find("+"), std::string::npos);
}

TEST_F(IRTest, GeneratesIfElseLabels) {
    std::string tac = GenerateIR("int main() { int x = 5; if (x > 0) { x = 1; } else { x = 0; } return 0; }");
    EXPECT_FALSE(tac.empty());
    // Should contain if_false, goto, and labels
    EXPECT_NE(tac.find("if_false"), std::string::npos);
    EXPECT_NE(tac.find("goto"), std::string::npos);
    EXPECT_NE(tac.find("L"), std::string::npos);
}

TEST_F(IRTest, GeneratesWhileLoop) {
    std::string tac = GenerateIR("int main() { int x = 0; while (x < 10) { x = x + 1; } return 0; }");
    EXPECT_FALSE(tac.empty());
    // While loop has: label at top, if_false, goto back
    size_t gotoCount = 0;
    for (size_t i = 0; (i = tac.find("goto", i)) != std::string::npos; i++) gotoCount++;
    EXPECT_GE(gotoCount, 2u); // at least if_false goto and unconditional goto
}

TEST_F(IRTest, GeneratesFunctionCall) {
    std::string tac = GenerateIR("int add(int a, int b) { return a + b; } int main() { int x = add(1, 2); return 0; }");
    EXPECT_FALSE(tac.empty());
    EXPECT_NE(tac.find("param"), std::string::npos);
    EXPECT_NE(tac.find("call"), std::string::npos);
}

TEST_F(IRTest, GeneratesReturnInstruction) {
    std::string tac = GenerateIR("int main() { return 0; }");
    EXPECT_FALSE(tac.empty());
    EXPECT_NE(tac.find("return"), std::string::npos);
}

TEST_F(IRTest, GeneratesWriteForCout) {
    std::string tac = GenerateIR("int main() { int x = 42; cout << x << endl; return 0; }");
    EXPECT_FALSE(tac.empty());
    EXPECT_NE(tac.find("write"), std::string::npos);
}
