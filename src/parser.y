/***************************************************************************
 * parser.y — Bison Grammar for SeedScan Mini-C++ Compiler
 * -------------------------------------------------------------------------
 * COMPILER THEORY — Syntax Analysis:
 * The parser is the SECOND phase. It takes tokens from the lexer and
 * verifies they form valid sentences according to the language grammar.
 * It builds an Abstract Syntax Tree (AST) as output.
 *
 * EXTENDED FROM: D:\Project\Project\parser.y
 * Original had flat printf rules. This version builds a full AST.
 ***************************************************************************/

%code requires {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <vector>
#include <string>
#include "seedscan/ast.hpp"

using namespace seedscan;

/* The parser's output: pointer to the root AST node */
extern seedscan::Program* g_program;

/* Flex/Bison interface */
extern int yylex();
extern int yylineno;
extern int yycolumn;
void yyerror(const char* s);
}

%{
/* Helper to set source location on an AST node */
#define SET_LOC(node) do { (node)->line = yylloc.first_line; (node)->column = yylloc.first_column; } while(0)
%}

/* ── Enable location tracking for error messages ───────────────────────── */
%locations

/* ── Semantic value union ──────────────────────────────────────────────── */
%union {
    int    ival;
    double fval;
    char   cval;
    int    bval;
    char*  str;

    seedscan::ASTNode*          node;
    seedscan::Program*          program;
    seedscan::VarDeclaration*   var_decl;
    seedscan::FunDeclaration*   fun_decl;
    seedscan::Parameter*        param;
    seedscan::CompoundStmt*     compound;
    seedscan::ClassDeclaration* class_decl;

    std::vector<seedscan::ASTNodePtr>*                  node_list;
    std::vector<std::unique_ptr<seedscan::Parameter>>*  param_list;
    std::vector<seedscan::ClassDeclaration::Member>*    member_list;
}

/* ── Token declarations ────────────────────────────────────────────────── */
%token <str>  ID STRING_LITERAL
%token <ival> INT_LITERAL
%token <fval> FLOAT_LITERAL
%token <cval> CHAR_LITERAL
%token <bval> TRUE_LIT FALSE_LIT

%token INT_TYPE FLOAT_TYPE CHAR_TYPE BOOL_TYPE VOID_TYPE
%token IF ELSE WHILE FOR RETURN
%token CLASS PUBLIC PRIVATE NEW
%token COUT CIN ENDL

%token PLUS MINUS STAR SLASH MOD
%token EQ NEQ LT GT LTE GTE
%token AND OR NOT
%token ASSIGN PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN SLASH_ASSIGN
%token INCREMENT DECREMENT
%token LSHIFT RSHIFT

%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET
%token SEMI COMMA DOT COLON
%token ERROR_TOKEN

/* ── Non-terminal types ────────────────────────────────────────────────── */
%type <program>     program
%type <node_list>   declaration_list statement_list
%type <node>        declaration var_declaration fun_declaration class_declaration
%type <node>        statement expression_stmt compound_stmt
%type <node>        selection_stmt iteration_stmt return_stmt io_stmt
%type <node>        expression assign_expr logical_or logical_and
%type <node>        equality relational additive multiplicative
%type <node>        unary_expr postfix_expr primary
%type <node>        for_init
%type <str>         type_specifier
%type <param_list>  param_list params
%type <param>       param
%type <node_list>   arg_list args cout_list cin_list
%type <member_list> class_body class_members
%type <node_list>   class_member_decls
%type <str>         access_specifier

/* ── Operator precedence (lowest to highest) ───────────────────────────
 * COMPILER THEORY: Bison resolves ambiguity using precedence. Operators
 * listed later have HIGHER precedence. %left means left-associative.
 *---------------------------------------------------------------------*/
%right ASSIGN PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN SLASH_ASSIGN
%left  OR
%left  AND
%left  EQ NEQ
%left  LT GT LTE GTE
%left  PLUS MINUS
%left  STAR SLASH MOD
%right NOT
%left  INCREMENT DECREMENT
%left  DOT
%left  LBRACKET

/* ── Dangling else resolution ──────────────────────────────────────────── */
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

/*=======================================================================
 * GRAMMAR RULES — Each rule has a semantic action that builds AST nodes.
 *=====================================================================*/

program:
    declaration_list {
        g_program = new Program(std::move(*$1));
        delete $1;
        $$ = g_program;
    }
    ;

declaration_list:
    declaration {
        $$ = new std::vector<ASTNodePtr>();
        if ($1) $$->push_back(ASTNodePtr($1));
    }
    | declaration_list declaration {
        $$ = $1;
        if ($2) $$->push_back(ASTNodePtr($2));
    }
    ;

declaration:
    var_declaration   { $$ = $1; }
    | fun_declaration { $$ = $1; }
    | class_declaration { $$ = $1; }
    ;

/* ── Type Specifiers ───────────────────────────────────────────────────── */
type_specifier:
    INT_TYPE    { $$ = strdup("int"); }
    | FLOAT_TYPE  { $$ = strdup("float"); }
    | CHAR_TYPE   { $$ = strdup("char"); }
    | BOOL_TYPE   { $$ = strdup("bool"); }
    | VOID_TYPE   { $$ = strdup("void"); }
    | ID          { $$ = $1; }
    ;

/* ── Variable Declaration ──────────────────────────────────────────────── */
var_declaration:
    type_specifier ID SEMI {
        $$ = new VarDeclaration($1, $2);
        SET_LOC($$);
        free($1); free($2);
    }
    | type_specifier ID ASSIGN expression SEMI {
        $$ = new VarDeclaration($1, $2, ASTNodePtr($4));
        SET_LOC($$);
        free($1); free($2);
    }
    | type_specifier ID LBRACKET INT_LITERAL RBRACKET SEMI {
        $$ = new ArrayDeclaration($1, $2, $4);
        SET_LOC($$);
        free($1); free($2);
    }
    ;

/* ── Function Declaration ──────────────────────────────────────────────── */
fun_declaration:
    type_specifier ID LPAREN params RPAREN compound_stmt {
        auto* fn = new FunDeclaration($1, $2, std::move(*$4), ASTNodePtr($6));
        SET_LOC(fn);
        $$ = fn;
        free($1); free($2); delete $4;
    }
    ;

params:
    param_list { $$ = $1; }
    | VOID_TYPE { $$ = new std::vector<std::unique_ptr<Parameter>>(); }
    | %empty   { $$ = new std::vector<std::unique_ptr<Parameter>>(); }
    ;

param_list:
    param {
        $$ = new std::vector<std::unique_ptr<Parameter>>();
        $$->push_back(std::unique_ptr<Parameter>($1));
    }
    | param_list COMMA param {
        $$ = $1;
        $$->push_back(std::unique_ptr<Parameter>($3));
    }
    ;

param:
    type_specifier ID {
        $$ = new Parameter($1, $2, false);
        SET_LOC($$);
        free($1); free($2);
    }
    | type_specifier ID LBRACKET RBRACKET {
        $$ = new Parameter($1, $2, true);
        SET_LOC($$);
        free($1); free($2);
    }
    ;

/* ── Class Declaration ─────────────────────────────────────────────────── */
class_declaration:
    CLASS ID LBRACE class_body RBRACE SEMI {
        auto* cls = new ClassDeclaration($2, std::move(*$4));
        SET_LOC(cls);
        $$ = cls;
        free($2); delete $4;
    }
    ;

class_body:
    class_members { $$ = $1; }
    | %empty { $$ = new std::vector<ClassDeclaration::Member>(); }
    ;

class_members:
    access_specifier COLON class_member_decls {
        /* $3 is a node_list, wrap each with access from $1 */
        $$ = new std::vector<ClassDeclaration::Member>();
        for (auto& n : *$3) {
            ClassDeclaration::Member m;
            m.access = $1;
            m.decl = std::move(n);
            $$->push_back(std::move(m));
        }
        free($1); delete $3;
    }
    | class_members access_specifier COLON class_member_decls {
        $$ = $1;
        for (auto& n : *$4) {
            ClassDeclaration::Member m;
            m.access = $2;
            m.decl = std::move(n);
            $$->push_back(std::move(m));
        }
        free($2); delete $4;
    }
    ;

class_member_decls:
    declaration {
        $$ = new std::vector<ASTNodePtr>();
        if ($1) $$->push_back(ASTNodePtr($1));
    }
    | class_member_decls declaration {
        $$ = $1;
        if ($2) $$->push_back(ASTNodePtr($2));
    }
    ;

access_specifier:
    PUBLIC  { $$ = strdup("public"); }
    | PRIVATE { $$ = strdup("private"); }
    ;

/* ── Compound Statement (Block) ────────────────────────────────────────── */
compound_stmt:
    LBRACE statement_list RBRACE {
        $$ = new CompoundStmt(std::move(*$2));
        SET_LOC($$);
        delete $2;
    }
    ;

statement_list:
    %empty {
        $$ = new std::vector<ASTNodePtr>();
    }
    | statement_list statement {
        $$ = $1;
        if ($2) $$->push_back(ASTNodePtr($2));
    }
    | statement_list var_declaration {
        $$ = $1;
        if ($2) $$->push_back(ASTNodePtr($2));
    }
    ;

statement:
    expression_stmt   { $$ = $1; }
    | compound_stmt   { $$ = $1; }
    | selection_stmt  { $$ = $1; }
    | iteration_stmt  { $$ = $1; }
    | return_stmt     { $$ = $1; }
    | io_stmt         { $$ = $1; }
    ;

expression_stmt:
    expression SEMI {
        $$ = new ExpressionStmt(ASTNodePtr($1));
        SET_LOC($$);
    }
    | SEMI { $$ = nullptr; }
    ;

/* ── If / Else ─────────────────────────────────────────────────────────── */
selection_stmt:
    IF LPAREN expression RPAREN statement %prec LOWER_THAN_ELSE {
        $$ = new IfStmt(ASTNodePtr($3), ASTNodePtr($5));
        SET_LOC($$);
    }
    | IF LPAREN expression RPAREN statement ELSE statement {
        $$ = new IfStmt(ASTNodePtr($3), ASTNodePtr($5), ASTNodePtr($7));
        SET_LOC($$);
    }
    ;

/* ── Loops ─────────────────────────────────────────────────────────────── */
iteration_stmt:
    WHILE LPAREN expression RPAREN statement {
        $$ = new WhileStmt(ASTNodePtr($3), ASTNodePtr($5));
        SET_LOC($$);
    }
    | FOR LPAREN for_init SEMI expression SEMI expression RPAREN statement {
        $$ = new ForStmt(ASTNodePtr($3), ASTNodePtr($5), ASTNodePtr($7), ASTNodePtr($9));
        SET_LOC($$);
    }
    ;

for_init:
    type_specifier ID ASSIGN expression {
        $$ = new VarDeclaration($1, $2, ASTNodePtr($4));
        SET_LOC($$);
        free($1); free($2);
    }
    | expression { $$ = $1; }
    | %empty { $$ = nullptr; }
    ;

/* ── Return ────────────────────────────────────────────────────────────── */
return_stmt:
    RETURN SEMI {
        $$ = new ReturnStmt();
        SET_LOC($$);
    }
    | RETURN expression SEMI {
        $$ = new ReturnStmt(ASTNodePtr($2));
        SET_LOC($$);
    }
    ;

/* ── I/O Statements ───────────────────────────────────────────────────── */
io_stmt:
    COUT cout_list SEMI {
        $$ = new CoutStmt(std::move(*$2));
        SET_LOC($$);
        delete $2;
    }
    | CIN cin_list SEMI {
        $$ = new CinStmt(std::move(*$2));
        SET_LOC($$);
        delete $2;
    }
    ;

cout_list:
    LSHIFT expression {
        $$ = new std::vector<ASTNodePtr>();
        $$->push_back(ASTNodePtr($2));
    }
    | LSHIFT ENDL {
        $$ = new std::vector<ASTNodePtr>();
        auto* nl = new StringLiteral("\\n");
        $$->push_back(ASTNodePtr(nl));
    }
    | cout_list LSHIFT expression {
        $$ = $1;
        $$->push_back(ASTNodePtr($3));
    }
    | cout_list LSHIFT ENDL {
        $$ = $1;
        auto* nl = new StringLiteral("\\n");
        $$->push_back(ASTNodePtr(nl));
    }
    ;

cin_list:
    RSHIFT primary {
        $$ = new std::vector<ASTNodePtr>();
        $$->push_back(ASTNodePtr($2));
    }
    | cin_list RSHIFT primary {
        $$ = $1;
        $$->push_back(ASTNodePtr($3));
    }
    ;

/* ── Expression Grammar (precedence climbing) ──────────────────────────
 * COMPILER THEORY: We split expression into layers, one per precedence
 * level. Each layer calls the next-higher-precedence layer. This
 * naturally encodes operator precedence in the grammar structure.
 *---------------------------------------------------------------------*/
expression:
    assign_expr { $$ = $1; }
    ;

assign_expr:
    logical_or { $$ = $1; }
    | postfix_expr ASSIGN assign_expr {
        $$ = new AssignExpr("=", ASTNodePtr($1), ASTNodePtr($3));
        SET_LOC($$);
    }
    | postfix_expr PLUS_ASSIGN assign_expr {
        $$ = new AssignExpr("+=", ASTNodePtr($1), ASTNodePtr($3));
        SET_LOC($$);
    }
    | postfix_expr MINUS_ASSIGN assign_expr {
        $$ = new AssignExpr("-=", ASTNodePtr($1), ASTNodePtr($3));
        SET_LOC($$);
    }
    | postfix_expr STAR_ASSIGN assign_expr {
        $$ = new AssignExpr("*=", ASTNodePtr($1), ASTNodePtr($3));
        SET_LOC($$);
    }
    | postfix_expr SLASH_ASSIGN assign_expr {
        $$ = new AssignExpr("/=", ASTNodePtr($1), ASTNodePtr($3));
        SET_LOC($$);
    }
    ;

logical_or:
    logical_and { $$ = $1; }
    | logical_or OR logical_and {
        $$ = new BinaryExpr("||", ASTNodePtr($1), ASTNodePtr($3));
        SET_LOC($$);
    }
    ;

logical_and:
    equality { $$ = $1; }
    | logical_and AND equality {
        $$ = new BinaryExpr("&&", ASTNodePtr($1), ASTNodePtr($3));
        SET_LOC($$);
    }
    ;

equality:
    relational { $$ = $1; }
    | equality EQ relational {
        $$ = new BinaryExpr("==", ASTNodePtr($1), ASTNodePtr($3));
        SET_LOC($$);
    }
    | equality NEQ relational {
        $$ = new BinaryExpr("!=", ASTNodePtr($1), ASTNodePtr($3));
        SET_LOC($$);
    }
    ;

relational:
    additive { $$ = $1; }
    | relational LT additive {
        $$ = new BinaryExpr("<", ASTNodePtr($1), ASTNodePtr($3));
        SET_LOC($$);
    }
    | relational GT additive {
        $$ = new BinaryExpr(">", ASTNodePtr($1), ASTNodePtr($3));
        SET_LOC($$);
    }
    | relational LTE additive {
        $$ = new BinaryExpr("<=", ASTNodePtr($1), ASTNodePtr($3));
        SET_LOC($$);
    }
    | relational GTE additive {
        $$ = new BinaryExpr(">=", ASTNodePtr($1), ASTNodePtr($3));
        SET_LOC($$);
    }
    ;

additive:
    multiplicative { $$ = $1; }
    | additive PLUS multiplicative {
        $$ = new BinaryExpr("+", ASTNodePtr($1), ASTNodePtr($3));
        SET_LOC($$);
    }
    | additive MINUS multiplicative {
        $$ = new BinaryExpr("-", ASTNodePtr($1), ASTNodePtr($3));
        SET_LOC($$);
    }
    ;

multiplicative:
    unary_expr { $$ = $1; }
    | multiplicative STAR unary_expr {
        $$ = new BinaryExpr("*", ASTNodePtr($1), ASTNodePtr($3));
        SET_LOC($$);
    }
    | multiplicative SLASH unary_expr {
        $$ = new BinaryExpr("/", ASTNodePtr($1), ASTNodePtr($3));
        SET_LOC($$);
    }
    | multiplicative MOD unary_expr {
        $$ = new BinaryExpr("%", ASTNodePtr($1), ASTNodePtr($3));
        SET_LOC($$);
    }
    ;

unary_expr:
    postfix_expr { $$ = $1; }
    | NOT unary_expr {
        $$ = new UnaryExpr("!", ASTNodePtr($2));
        SET_LOC($$);
    }
    | MINUS unary_expr {
        $$ = new UnaryExpr("-", ASTNodePtr($2));
        SET_LOC($$);
    }
    | INCREMENT postfix_expr {
        $$ = new UnaryExpr("++", ASTNodePtr($2));
        SET_LOC($$);
    }
    | DECREMENT postfix_expr {
        $$ = new UnaryExpr("--", ASTNodePtr($2));
        SET_LOC($$);
    }
    ;

postfix_expr:
    primary { $$ = $1; }
    | postfix_expr INCREMENT {
        $$ = new PostfixExpr("++", ASTNodePtr($1));
        SET_LOC($$);
    }
    | postfix_expr DECREMENT {
        $$ = new PostfixExpr("--", ASTNodePtr($1));
        SET_LOC($$);
    }
    | ID LPAREN args RPAREN {
        $$ = new CallExpr($1, std::move(*$3));
        SET_LOC($$);
        free($1); delete $3;
    }
    | postfix_expr DOT ID {
        $$ = new MemberAccessExpr(ASTNodePtr($1), $3);
        SET_LOC($$);
        free($3);
    }
    | postfix_expr DOT ID LPAREN args RPAREN {
        $$ = new MethodCallExpr(ASTNodePtr($1), $3, std::move(*$5));
        SET_LOC($$);
        free($3); delete $5;
    }
    | ID LBRACKET expression RBRACKET {
        $$ = new ArrayAccessExpr($1, ASTNodePtr($3));
        SET_LOC($$);
        free($1);
    }
    ;

primary:
    INT_LITERAL {
        $$ = new IntLiteral($1);
        SET_LOC($$);
    }
    | FLOAT_LITERAL {
        $$ = new FloatLiteral($1);
        SET_LOC($$);
    }
    | CHAR_LITERAL {
        $$ = new CharLiteral($1);
        SET_LOC($$);
    }
    | STRING_LITERAL {
        $$ = new StringLiteral($1);
        SET_LOC($$);
        free($1);
    }
    | TRUE_LIT {
        $$ = new BoolLiteral(true);
        SET_LOC($$);
    }
    | FALSE_LIT {
        $$ = new BoolLiteral(false);
        SET_LOC($$);
    }
    | ID {
        $$ = new IdentifierExpr($1);
        SET_LOC($$);
        free($1);
    }
    | LPAREN expression RPAREN {
        $$ = $2;
    }
    ;

args:
    arg_list { $$ = $1; }
    | %empty { $$ = new std::vector<ASTNodePtr>(); }
    ;

arg_list:
    expression {
        $$ = new std::vector<ASTNodePtr>();
        $$->push_back(ASTNodePtr($1));
    }
    | arg_list COMMA expression {
        $$ = $1;
        $$->push_back(ASTNodePtr($3));
    }
    ;

%%

/* ── Error handler ─────────────────────────────────────────────────────── */
void yyerror(const char* s) {
    fprintf(stderr, "Parse Error at line %d, col %d: %s\n",
            yylloc.first_line, yylloc.first_column, s);
}
