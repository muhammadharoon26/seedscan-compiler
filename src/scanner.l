/***************************************************************************
 * scanner.l — Flex Lexer Specification for SeedScan Mini-C++ Compiler
 * -------------------------------------------------------------------------
 * COMPILER THEORY — Lexical Analysis (Scanning):
 * The lexer is the FIRST phase of compilation. It reads raw source code
 * character by character and groups them into TOKENS — the smallest
 * meaningful units of the language.
 *
 * Flex works by matching input against regular expression rules (longest
 * match wins). Each rule returns a token type to the parser.
 *
 * EXTENDED FROM: D:\Project\Project\scanner.l
 * Original had 6 keywords and 6 operators with intentionally flipped
 * mappings. This version has correct mappings and full Mini-C++ coverage.
 *
 * KEY CHANGES FROM ORIGINAL:
 *   - Fixed: "int" now correctly returns INT_TYPE (was FLOAT_TYPE)
 *   - Fixed: "+" now correctly returns PLUS (was MINUS_OP)
 *   - Added: 13 new keywords (float, char, bool, void, if, return, etc.)
 *   - Added: 15 new operators (==, !=, <=, >=, &&, ||, ++, --, etc.)
 *   - Added: Float, char, string literal recognition
 *   - Added: Comment handling (// and /* *\/)
 *   - Added: Line/column tracking for error reporting
 ***************************************************************************/

%{
/*------------------------------------------------------------------------
 * C/C++ PROLOGUE — Code inserted verbatim at the top of the generated
 * scanner. This is where we include headers and declare helper variables.
 *----------------------------------------------------------------------*/
#include "parser.hpp"          /* Bison-generated token definitions          */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Column tracking ──────────────────────────────────────────────────
 * Flex tracks lines automatically via %option yylineno, but columns
 * must be tracked manually. We update column on every token match and
 * reset it on newlines.
 *--------------------------------------------------------------------*/
int yycolumn = 1;

/* YY_USER_ACTION runs before every rule action. We use it to update
 * the column counter based on the length of the matched text (yyleng). */
#define YY_USER_ACTION \
    yylloc.first_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_line = yylineno; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;
%}

/*------------------------------------------------------------------------
 * FLEX OPTIONS
 *   noyywrap  — Don't call yywrap() at EOF (single-file compilation)
 *   yylineno  — Automatically track line numbers in yylineno
 *   nounput   — Suppress unused function warning
 *   noinput   — Suppress unused function warning
 *----------------------------------------------------------------------*/
%option noyywrap
%option yylineno
%option nounput
%option noinput

/*------------------------------------------------------------------------
 * FLEX START CONDITIONS
 *   COMMENT — Exclusive state for handling multi-line /* *\/ comments.
 *   When in COMMENT state, only comment-related rules are active.
 *----------------------------------------------------------------------*/
%x COMMENT

%%
 /*=======================================================================
  * SECTION 2: LEXER RULES
  * Each rule is a regex pattern followed by an action (C code).
  * Flex matches the LONGEST possible input; ties go to the first rule.
  *=====================================================================*/

 /*── Multi-line comment handling ────────────────────────────────────────
  * COMPILER THEORY: Multi-line comments require a "start condition" in
  * Flex because they span multiple lines and can't be matched by a
  * single regex efficiently. We enter COMMENT state on "/*" and exit
  * on "*\/", discarding everything in between.
  *--------------------------------------------------------------------*/
"/*"                { BEGIN(COMMENT); }
<COMMENT>"*/"       { BEGIN(INITIAL); }
<COMMENT>\n         { yycolumn = 1; }
<COMMENT>.          { /* discard comment content */ }

 /*── Single-line comments ──────────────────────────────────────────────*/
"//".*              { /* discard single-line comment */ }

 /*── Keywords ──────────────────────────────────────────────────────────
  * COMPILER THEORY: Keywords are reserved words that cannot be used as
  * identifiers. They MUST be matched BEFORE the identifier rule, because
  * Flex returns the first matching rule on ties. Since keywords are also
  * valid identifier patterns, placing them first ensures correct matching.
  *--------------------------------------------------------------------*/
"int"               { return INT_TYPE; }
"float"             { return FLOAT_TYPE; }
"char"              { return CHAR_TYPE; }
"bool"              { return BOOL_TYPE; }
"void"              { return VOID_TYPE; }
"if"                { return IF; }
"else"              { return ELSE; }
"while"             { return WHILE; }
"for"               { return FOR; }
"return"            { return RETURN; }
"class"             { return CLASS; }
"public"            { return PUBLIC; }
"private"           { return PRIVATE; }
"true"              { yylval.bval = 1; return TRUE_LIT; }
"false"             { yylval.bval = 0; return FALSE_LIT; }
"cout"              { return COUT; }
"cin"               { return CIN; }
"endl"              { return ENDL; }
"new"               { return NEW; }

 /*── Two-character operators ───────────────────────────────────────────
  * IMPORTANT: These MUST come before single-character operators.
  * Flex's longest-match rule handles this naturally, but explicit
  * ordering makes the intent clear.
  *--------------------------------------------------------------------*/
"=="                { return EQ; }
"!="                { return NEQ; }
"<="                { return LTE; }
">="                { return GTE; }
"&&"                { return AND; }
"||"                { return OR; }
"++"                { return INCREMENT; }
"--"                { return DECREMENT; }
"+="                { return PLUS_ASSIGN; }
"-="                { return MINUS_ASSIGN; }
"*="                { return STAR_ASSIGN; }
"/="                { return SLASH_ASSIGN; }
"<<"                { return LSHIFT; }
">>"                { return RSHIFT; }

 /*── Single-character operators ────────────────────────────────────────*/
"+"                 { return PLUS; }
"-"                 { return MINUS; }
"*"                 { return STAR; }
"/"                 { return SLASH; }
"%"                 { return MOD; }
"="                 { return ASSIGN; }
"<"                 { return LT; }
">"                 { return GT; }
"!"                 { return NOT; }

 /*── Delimiters ────────────────────────────────────────────────────────*/
"("                 { return LPAREN; }
")"                 { return RPAREN; }
"{"                 { return LBRACE; }
"}"                 { return RBRACE; }
"["                 { return LBRACKET; }
"]"                 { return RBRACKET; }
";"                 { return SEMI; }
","                 { return COMMA; }
"."                 { return DOT; }
":"                 { return COLON; }

 /*── Literals ──────────────────────────────────────────────────────────
  * COMPILER THEORY: Literal recognition uses regex patterns to match
  * numeric, character, and string constants. The matched text (yytext)
  * is converted to the appropriate C type and stored in yylval (the
  * semantic value union shared with Bison).
  *--------------------------------------------------------------------*/

 /* Float literals: must match before int to handle "3.14" correctly */
[0-9]+\.[0-9]+      {
    yylval.fval = atof(yytext);
    return FLOAT_LITERAL;
}

 /* Integer literals */
[0-9]+              {
    yylval.ival = atoi(yytext);
    return INT_LITERAL;
}

 /* Character literals: single character in single quotes */
\'([^'\\]|\\.)\' {
    /* Handle escape sequences like '\n', '\t' */
    if (yytext[1] == '\\') {
        switch (yytext[2]) {
            case 'n':  yylval.cval = '\n'; break;
            case 't':  yylval.cval = '\t'; break;
            case '\\': yylval.cval = '\\'; break;
            case '\'': yylval.cval = '\''; break;
            case '0':  yylval.cval = '\0'; break;
            default:   yylval.cval = yytext[2]; break;
        }
    } else {
        yylval.cval = yytext[1];
    }
    return CHAR_LITERAL;
}

 /* String literals: text in double quotes (used in cout) */
\"([^"\\]|\\.)*\"  {
    /* Strip the surrounding quotes and store the content */
    int len = strlen(yytext) - 2;
    yylval.str = (char*)malloc(len + 1);
    strncpy(yylval.str, yytext + 1, len);
    yylval.str[len] = '\0';
    return STRING_LITERAL;
}

 /*── Identifiers ───────────────────────────────────────────────────────
  * COMPILER THEORY: An identifier is a user-defined name for variables,
  * functions, or classes. It starts with a letter or underscore,
  * followed by letters, digits, or underscores. This rule MUST come
  * AFTER all keyword rules (Flex longest-match + first-rule tiebreaker).
  *--------------------------------------------------------------------*/
[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.str = strdup(yytext);
    return ID;
}

 /*── Whitespace handling ───────────────────────────────────────────────*/
\n                  { yycolumn = 1; /* Reset column on newline */ }
[ \t\r]+            { /* Skip spaces, tabs, carriage returns */ }

 /*── Error handling ────────────────────────────────────────────────────
  * Any character not matched by the above rules is an error.
  * We return it as an error token rather than silently ignoring it
  * (unlike the original scanner.l which used "." to ignore).
  *--------------------------------------------------------------------*/
.                   {
    fprintf(stderr, "Lexer Error at line %d, col %d: Unexpected character '%s'\n",
            yylineno, yycolumn - 1, yytext);
    return ERROR_TOKEN;
}

%%
