/***************************************************************************
 * semantic_test.cpp — Unit Tests for the Semantic Analyzer
 * -------------------------------------------------------------------------
 * Tests type checking, scope resolution, and error detection.
 ***************************************************************************/
#include <gtest/gtest.h>
#include <cstdio>
#include "seedscan/ast.hpp"
#include "seedscan/semantic_analyzer.hpp"
#include "seedscan/error_handler.hpp"

extern FILE* yyin;
extern int   yyparse();
extern int   yylineno;
extern int   yycolumn;
extern seedscan::Program* g_program;

class SemanticTest : public ::testing::Test {
protected:
    seedscan::ErrorHandler errors_;
    FILE* tempFile_ = nullptr;

    bool AnalyzeInput(const std::string& code) {
        tempFile_ = tmpfile();
        if (!tempFile_) return false;
        fputs(code.c_str(), tempFile_);
        rewind(tempFile_);
        yyin = tempFile_;
        yylineno = 1; yycolumn = 1;
        g_program = nullptr;
        if (yyparse() != 0 || !g_program) return false;

        errors_.clear();
        seedscan::SemanticAnalyzer analyzer(errors_);
        analyzer.analyze(*g_program);
        return true;
    }

    void TearDown() override {
        if (tempFile_) fclose(tempFile_);
        if (g_program) { delete g_program; g_program = nullptr; }
    }
};

// ── Scope Tests ───────────────────────────────────────────────────────────
TEST_F(SemanticTest, DetectsUndeclaredVariable) {
    AnalyzeInput("int main() { x = 10; return 0; }");
    EXPECT_TRUE(errors_.hasErrors());
}

TEST_F(SemanticTest, AcceptsDeclaredVariable) {
    AnalyzeInput("int main() { int x = 10; return 0; }");
    EXPECT_FALSE(errors_.hasErrors());
}

TEST_F(SemanticTest, DetectsRedeclarationInSameScope) {
    AnalyzeInput("int main() { int x = 1; int x = 2; return 0; }");
    EXPECT_TRUE(errors_.hasErrors());
}

TEST_F(SemanticTest, AllowsShadowingInNestedScope) {
    AnalyzeInput("int main() { int x = 1; { int x = 2; } return 0; }");
    EXPECT_FALSE(errors_.hasErrors());
}

// ── Type Checking Tests ───────────────────────────────────────────────────
TEST_F(SemanticTest, AcceptsIntArithmetic) {
    AnalyzeInput("int main() { int a = 1; int b = 2; int c = a + b; return 0; }");
    EXPECT_FALSE(errors_.hasErrors());
}

TEST_F(SemanticTest, AcceptsIntFloatPromotion) {
    AnalyzeInput("int main() { int a = 1; float b = 2.0; float c = a + b; return 0; }");
    EXPECT_FALSE(errors_.hasErrors());
}

// ── Function Tests ────────────────────────────────────────────────────────
TEST_F(SemanticTest, DetectsUndeclaredFunction) {
    AnalyzeInput("int main() { int x = foo(); return 0; }");
    EXPECT_TRUE(errors_.hasErrors());
}

TEST_F(SemanticTest, DetectsWrongArgCount) {
    AnalyzeInput("int add(int a, int b) { return a + b; } int main() { int x = add(1); return 0; }");
    EXPECT_TRUE(errors_.hasErrors());
}

TEST_F(SemanticTest, AcceptsCorrectFunctionCall) {
    AnalyzeInput("int add(int a, int b) { return a + b; } int main() { int x = add(1, 2); return 0; }");
    EXPECT_FALSE(errors_.hasErrors());
}

// ── Return Type Tests ─────────────────────────────────────────────────────
TEST_F(SemanticTest, DetectsReturnInVoidFunction) {
    AnalyzeInput("void foo() { return 42; } int main() { return 0; }");
    EXPECT_TRUE(errors_.hasErrors());
}

// ── Array Tests ───────────────────────────────────────────────────────────
TEST_F(SemanticTest, AcceptsArrayAccess) {
    AnalyzeInput("int main() { int arr[5]; arr[0] = 10; return 0; }");
    EXPECT_FALSE(errors_.hasErrors());
}

// ── Valid Complete Program ─────────────────────────────────────────────────
TEST_F(SemanticTest, AcceptsCompleteValidProgram) {
    AnalyzeInput(R"(
        int factorial(int n) {
            if (n <= 1) { return 1; }
            return n * factorial(n - 1);
        }
        int main() {
            int result = factorial(5);
            cout << result << endl;
            return 0;
        }
    )");
    EXPECT_FALSE(errors_.hasErrors());
}
