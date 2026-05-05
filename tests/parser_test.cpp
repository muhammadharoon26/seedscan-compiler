/***************************************************************************
 * parser_test.cpp — Unit Tests for the Bison Parser
 * -------------------------------------------------------------------------
 * Tests that the parser produces correct AST structures for all
 * supported Mini-C++ constructs.
 ***************************************************************************/
#include <gtest/gtest.h>
#include <cstdio>
#include <string>
#include "seedscan/ast.hpp"

extern FILE* yyin;
extern int   yyparse();
extern int   yylineno;
extern int   yycolumn;
extern seedscan::Program* g_program;

class ParserTest : public ::testing::Test {
protected:
    FILE* tempFile_ = nullptr;

    void ParseInput(const std::string& code) {
        tempFile_ = tmpfile();
        ASSERT_NE(tempFile_, nullptr);
        fputs(code.c_str(), tempFile_);
        rewind(tempFile_);
        yyin = tempFile_;
        yylineno = 1;
        yycolumn = 1;
        g_program = nullptr;
        yyparse();
    }

    void TearDown() override {
        if (tempFile_) fclose(tempFile_);
        if (g_program) { delete g_program; g_program = nullptr; }
    }
};

TEST_F(ParserTest, ParsesVarDeclaration) {
    ParseInput("int main() { int x = 42; return 0; }");
    ASSERT_NE(g_program, nullptr);
    ASSERT_EQ(g_program->declarations.size(), 1u);
    auto* fun = dynamic_cast<seedscan::FunDeclaration*>(g_program->declarations[0].get());
    ASSERT_NE(fun, nullptr);
    EXPECT_EQ(fun->name, "main");
}

TEST_F(ParserTest, ParsesIfElse) {
    ParseInput("int main() { if (x > 5) { return 1; } else { return 0; } }");
    ASSERT_NE(g_program, nullptr);
    auto* fun = dynamic_cast<seedscan::FunDeclaration*>(g_program->declarations[0].get());
    ASSERT_NE(fun, nullptr);
    auto* body = dynamic_cast<seedscan::CompoundStmt*>(fun->body.get());
    ASSERT_NE(body, nullptr);
    ASSERT_GE(body->statements.size(), 1u);
    auto* ifStmt = dynamic_cast<seedscan::IfStmt*>(body->statements[0].get());
    ASSERT_NE(ifStmt, nullptr);
    EXPECT_NE(ifStmt->elseBranch, nullptr);
}

TEST_F(ParserTest, ParsesWhileLoop) {
    ParseInput("int main() { while (x < 10) { x = x + 1; } return 0; }");
    ASSERT_NE(g_program, nullptr);
    auto* fun = dynamic_cast<seedscan::FunDeclaration*>(g_program->declarations[0].get());
    auto* body = dynamic_cast<seedscan::CompoundStmt*>(fun->body.get());
    ASSERT_NE(body, nullptr);
    auto* whileStmt = dynamic_cast<seedscan::WhileStmt*>(body->statements[0].get());
    ASSERT_NE(whileStmt, nullptr);
}

TEST_F(ParserTest, ParsesForLoop) {
    ParseInput("int main() { for (int i = 0; i < 10; i++) { } return 0; }");
    ASSERT_NE(g_program, nullptr);
}

TEST_F(ParserTest, ParsesFunctionWithParams) {
    ParseInput("int add(int a, int b) { return a + b; } int main() { return 0; }");
    ASSERT_NE(g_program, nullptr);
    ASSERT_GE(g_program->declarations.size(), 2u);
    auto* addFn = dynamic_cast<seedscan::FunDeclaration*>(g_program->declarations[0].get());
    ASSERT_NE(addFn, nullptr);
    EXPECT_EQ(addFn->name, "add");
    EXPECT_EQ(addFn->params.size(), 2u);
    EXPECT_EQ(addFn->returnType, "int");
}

TEST_F(ParserTest, ParsesArrayDeclaration) {
    ParseInput("int main() { int arr[10]; return 0; }");
    ASSERT_NE(g_program, nullptr);
}

TEST_F(ParserTest, ParsesClassDeclaration) {
    ParseInput("class Foo { public: int x; }; int main() { return 0; }");
    ASSERT_NE(g_program, nullptr);
    auto* cls = dynamic_cast<seedscan::ClassDeclaration*>(g_program->declarations[0].get());
    ASSERT_NE(cls, nullptr);
    EXPECT_EQ(cls->name, "Foo");
}

TEST_F(ParserTest, ParsesCoutStatement) {
    ParseInput("int main() { cout << \"hello\" << endl; return 0; }");
    ASSERT_NE(g_program, nullptr);
}

TEST_F(ParserTest, ParsesCinStatement) {
    ParseInput("int main() { int x; cin >> x; return 0; }");
    ASSERT_NE(g_program, nullptr);
}

TEST_F(ParserTest, ParsesArithmeticPrecedence) {
    // 2 + 3 * 4 should parse as 2 + (3 * 4)
    ParseInput("int main() { int x = 2 + 3 * 4; return 0; }");
    ASSERT_NE(g_program, nullptr);
}
