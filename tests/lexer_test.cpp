/***************************************************************************
 * lexer_test.cpp — Unit Tests for the Flex Lexer
 * -------------------------------------------------------------------------
 * Tests token recognition for all Mini-C++ lexical elements:
 * keywords, operators, literals, identifiers, and error tokens.
 ***************************************************************************/
#include <gtest/gtest.h>
#include <cstdio>
#include <cstring>
#include <string>

// Flex/Bison interface
extern FILE* yyin;
extern int   yylex();
extern char* yytext;
extern int   yylineno;
extern int   yycolumn;

// Include the Bison-generated token definitions
#include "parser.hpp"

// Bison semantic value
extern YYSTYPE yylval;
extern YYLTYPE yylloc;

// Helper: set up lexer to scan a string via a temp file
class LexerTest : public ::testing::Test {
protected:
    FILE* tempFile_ = nullptr;

    void SetInput(const std::string& code) {
        tempFile_ = tmpfile();
        ASSERT_NE(tempFile_, nullptr);
        fputs(code.c_str(), tempFile_);
        rewind(tempFile_);
        yyin = tempFile_;
        yylineno = 1;
        yycolumn = 1;
    }

    void TearDown() override {
        if (tempFile_) fclose(tempFile_);
    }
};

// ── Keyword Tests ─────────────────────────────────────────────────────────
TEST_F(LexerTest, RecognizesIntKeyword) {
    SetInput("int");
    EXPECT_EQ(yylex(), INT_TYPE);
}

TEST_F(LexerTest, RecognizesFloatKeyword) {
    SetInput("float");
    EXPECT_EQ(yylex(), FLOAT_TYPE);
}

TEST_F(LexerTest, RecognizesCharKeyword) {
    SetInput("char");
    EXPECT_EQ(yylex(), CHAR_TYPE);
}

TEST_F(LexerTest, RecognizesBoolKeyword) {
    SetInput("bool");
    EXPECT_EQ(yylex(), BOOL_TYPE);
}

TEST_F(LexerTest, RecognizesVoidKeyword) {
    SetInput("void");
    EXPECT_EQ(yylex(), VOID_TYPE);
}

TEST_F(LexerTest, RecognizesControlFlowKeywords) {
    SetInput("if else while for return");
    EXPECT_EQ(yylex(), IF);
    EXPECT_EQ(yylex(), ELSE);
    EXPECT_EQ(yylex(), WHILE);
    EXPECT_EQ(yylex(), FOR);
    EXPECT_EQ(yylex(), RETURN);
}

TEST_F(LexerTest, RecognizesClassKeywords) {
    SetInput("class public private");
    EXPECT_EQ(yylex(), CLASS);
    EXPECT_EQ(yylex(), PUBLIC);
    EXPECT_EQ(yylex(), PRIVATE);
}

TEST_F(LexerTest, RecognizesIOKeywords) {
    SetInput("cout cin endl");
    EXPECT_EQ(yylex(), COUT);
    EXPECT_EQ(yylex(), CIN);
    EXPECT_EQ(yylex(), ENDL);
}

TEST_F(LexerTest, RecognizesBoolLiterals) {
    SetInput("true false");
    EXPECT_EQ(yylex(), TRUE_LIT);
    EXPECT_EQ(yylex(), FALSE_LIT);
}

// ── Operator Tests ────────────────────────────────────────────────────────
TEST_F(LexerTest, RecognizesArithmeticOperators) {
    SetInput("+ - * / %");
    EXPECT_EQ(yylex(), PLUS);
    EXPECT_EQ(yylex(), MINUS);
    EXPECT_EQ(yylex(), STAR);
    EXPECT_EQ(yylex(), SLASH);
    EXPECT_EQ(yylex(), MOD);
}

TEST_F(LexerTest, RecognizesComparisonOperators) {
    SetInput("== != < > <= >=");
    EXPECT_EQ(yylex(), EQ);
    EXPECT_EQ(yylex(), NEQ);
    EXPECT_EQ(yylex(), LT);
    EXPECT_EQ(yylex(), GT);
    EXPECT_EQ(yylex(), LTE);
    EXPECT_EQ(yylex(), GTE);
}

TEST_F(LexerTest, RecognizesLogicalOperators) {
    SetInput("&& || !");
    EXPECT_EQ(yylex(), AND);
    EXPECT_EQ(yylex(), OR);
    EXPECT_EQ(yylex(), NOT);
}

TEST_F(LexerTest, RecognizesAssignmentOperators) {
    SetInput("= += -= *= /=");
    EXPECT_EQ(yylex(), ASSIGN);
    EXPECT_EQ(yylex(), PLUS_ASSIGN);
    EXPECT_EQ(yylex(), MINUS_ASSIGN);
    EXPECT_EQ(yylex(), STAR_ASSIGN);
    EXPECT_EQ(yylex(), SLASH_ASSIGN);
}

TEST_F(LexerTest, RecognizesIncrementDecrement) {
    SetInput("++ --");
    EXPECT_EQ(yylex(), INCREMENT);
    EXPECT_EQ(yylex(), DECREMENT);
}

TEST_F(LexerTest, RecognizesStreamOperators) {
    SetInput("<< >>");
    EXPECT_EQ(yylex(), LSHIFT);
    EXPECT_EQ(yylex(), RSHIFT);
}

// ── Literal Tests ─────────────────────────────────────────────────────────
TEST_F(LexerTest, RecognizesIntegerLiterals) {
    SetInput("42 0 1000");
    EXPECT_EQ(yylex(), INT_LITERAL);
    EXPECT_EQ(yylval.ival, 42);
    EXPECT_EQ(yylex(), INT_LITERAL);
    EXPECT_EQ(yylval.ival, 0);
    EXPECT_EQ(yylex(), INT_LITERAL);
    EXPECT_EQ(yylval.ival, 1000);
}

TEST_F(LexerTest, RecognizesFloatLiterals) {
    SetInput("3.14 0.5");
    EXPECT_EQ(yylex(), FLOAT_LITERAL);
    EXPECT_NEAR(yylval.fval, 3.14, 0.001);
    EXPECT_EQ(yylex(), FLOAT_LITERAL);
    EXPECT_NEAR(yylval.fval, 0.5, 0.001);
}

TEST_F(LexerTest, RecognizesCharLiterals) {
    SetInput("'a'");
    EXPECT_EQ(yylex(), CHAR_LITERAL);
    EXPECT_EQ(yylval.cval, 'a');
}

TEST_F(LexerTest, RecognizesStringLiterals) {
    SetInput("\"hello\"");
    EXPECT_EQ(yylex(), STRING_LITERAL);
    EXPECT_STREQ(yylval.str, "hello");
    free(yylval.str);
}

// ── Identifier Tests ──────────────────────────────────────────────────────
TEST_F(LexerTest, RecognizesIdentifiers) {
    SetInput("myVar _temp counter123");
    EXPECT_EQ(yylex(), ID);
    EXPECT_STREQ(yylval.str, "myVar");
    free(yylval.str);
    EXPECT_EQ(yylex(), ID);
    EXPECT_STREQ(yylval.str, "_temp");
    free(yylval.str);
    EXPECT_EQ(yylex(), ID);
    EXPECT_STREQ(yylval.str, "counter123");
    free(yylval.str);
}

// ── Delimiter Tests ───────────────────────────────────────────────────────
TEST_F(LexerTest, RecognizesDelimiters) {
    SetInput("( ) { } [ ] ; , . :");
    EXPECT_EQ(yylex(), LPAREN);
    EXPECT_EQ(yylex(), RPAREN);
    EXPECT_EQ(yylex(), LBRACE);
    EXPECT_EQ(yylex(), RBRACE);
    EXPECT_EQ(yylex(), LBRACKET);
    EXPECT_EQ(yylex(), RBRACKET);
    EXPECT_EQ(yylex(), SEMI);
    EXPECT_EQ(yylex(), COMMA);
    EXPECT_EQ(yylex(), DOT);
    EXPECT_EQ(yylex(), COLON);
}

// ── Comment Tests ─────────────────────────────────────────────────────────
TEST_F(LexerTest, SkipsSingleLineComments) {
    SetInput("int // this is a comment\nfloat");
    EXPECT_EQ(yylex(), INT_TYPE);
    EXPECT_EQ(yylex(), FLOAT_TYPE);
}

TEST_F(LexerTest, SkipsMultiLineComments) {
    SetInput("int /* multi\nline\ncomment */ float");
    EXPECT_EQ(yylex(), INT_TYPE);
    EXPECT_EQ(yylex(), FLOAT_TYPE);
}

// ── Complete Statement Test ───────────────────────────────────────────────
TEST_F(LexerTest, TokenizesCompleteStatement) {
    SetInput("int x = 42;");
    EXPECT_EQ(yylex(), INT_TYPE);
    EXPECT_EQ(yylex(), ID);
    free(yylval.str);
    EXPECT_EQ(yylex(), ASSIGN);
    EXPECT_EQ(yylex(), INT_LITERAL);
    EXPECT_EQ(yylval.ival, 42);
    EXPECT_EQ(yylex(), SEMI);
}
