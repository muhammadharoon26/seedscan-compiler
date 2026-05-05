/***************************************************************************
 * ast.hpp — Abstract Syntax Tree Node Hierarchy for Mini-C++
 * -------------------------------------------------------------------------
 * COMPILER THEORY:
 * An Abstract Syntax Tree (AST) is the core data structure produced by the
 * parser. Unlike a parse tree (which mirrors the grammar), an AST captures
 * only the essential structure of the program, discarding syntactic sugar
 * like parentheses and semicolons.
 *
 * DESIGN PATTERN — Visitor:
 * We use the Visitor pattern to separate the tree structure from the
 * operations performed on it. This lets us add new passes (semantic
 * analysis, IR generation, pretty-printing) without modifying node classes.
 *
 * MEMORY MANAGEMENT:
 * All child nodes are owned via std::unique_ptr for automatic cleanup.
 * The parser creates nodes with 'new' and wraps them in unique_ptr.
 ***************************************************************************/
#ifndef SEEDSCAN_AST_HPP
#define SEEDSCAN_AST_HPP

#include <string>
#include <vector>
#include <memory>

namespace seedscan {

// Forward-declare the visitor interface so nodes can accept visitors
class ASTVisitor;

/*==========================================================================
 * BASE CLASS — All AST nodes inherit from this
 *========================================================================*/
class ASTNode {
public:
    int line = 0;       // Source line for error reporting
    int column = 0;     // Source column for error reporting

    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
};

// Convenience typedef for owned node pointers
using ASTNodePtr = std::unique_ptr<ASTNode>;

/*==========================================================================
 * EXPRESSION NODES — Represent values and computations
 *========================================================================*/

/*--------------------------------------------------------------------------
 * IntLiteral — Integer constant, e.g., 42
 *------------------------------------------------------------------------*/
class IntLiteral : public ASTNode {
public:
    int value;
    explicit IntLiteral(int val) : value(val) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * FloatLiteral — Floating-point constant, e.g., 3.14
 *------------------------------------------------------------------------*/
class FloatLiteral : public ASTNode {
public:
    double value;
    explicit FloatLiteral(double val) : value(val) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * CharLiteral — Character constant, e.g., 'a'
 *------------------------------------------------------------------------*/
class CharLiteral : public ASTNode {
public:
    char value;
    explicit CharLiteral(char val) : value(val) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * StringLiteral — String constant, e.g., "hello" (used in cout only)
 *------------------------------------------------------------------------*/
class StringLiteral : public ASTNode {
public:
    std::string value;
    explicit StringLiteral(const std::string& val) : value(val) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * BoolLiteral — Boolean constant: true or false
 *------------------------------------------------------------------------*/
class BoolLiteral : public ASTNode {
public:
    bool value;
    explicit BoolLiteral(bool val) : value(val) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * IdentifierExpr — A variable reference, e.g., x, myVar
 * This is an L-value: it can appear on the left side of assignment.
 *------------------------------------------------------------------------*/
class IdentifierExpr : public ASTNode {
public:
    std::string name;
    explicit IdentifierExpr(const std::string& n) : name(n) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * BinaryExpr — Binary operation: left OP right
 * Covers arithmetic (+,-,*,/,%), comparison (==,!=,<,>,<=,>=),
 * and logical (&&, ||).
 *------------------------------------------------------------------------*/
class BinaryExpr : public ASTNode {
public:
    std::string op;         // Operator as string: "+", "==", "&&", etc.
    ASTNodePtr  left;
    ASTNodePtr  right;

    BinaryExpr(const std::string& op, ASTNodePtr l, ASTNodePtr r)
        : op(op), left(std::move(l)), right(std::move(r)) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * UnaryExpr — Prefix unary operation: OP expr
 * Covers: !, - (negation), ++x, --x
 *------------------------------------------------------------------------*/
class UnaryExpr : public ASTNode {
public:
    std::string op;
    ASTNodePtr  operand;

    UnaryExpr(const std::string& op, ASTNodePtr operand)
        : op(op), operand(std::move(operand)) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * PostfixExpr — Postfix operation: expr OP
 * Covers: x++, x--
 *------------------------------------------------------------------------*/
class PostfixExpr : public ASTNode {
public:
    std::string op;
    ASTNodePtr  operand;

    PostfixExpr(const std::string& op, ASTNodePtr operand)
        : op(op), operand(std::move(operand)) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * AssignExpr — Assignment: target = value
 * Also covers compound assignment: +=, -=, *=, /=
 *------------------------------------------------------------------------*/
class AssignExpr : public ASTNode {
public:
    std::string op;         // "=", "+=", "-=", "*=", "/="
    ASTNodePtr  target;     // Must be an L-value (IdentifierExpr or ArrayAccessExpr)
    ASTNodePtr  value;

    AssignExpr(const std::string& op, ASTNodePtr target, ASTNodePtr value)
        : op(op), target(std::move(target)), value(std::move(value)) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * CallExpr — Function call: name(arg1, arg2, ...)
 *------------------------------------------------------------------------*/
class CallExpr : public ASTNode {
public:
    std::string                name;
    std::vector<ASTNodePtr>    args;

    CallExpr(const std::string& n, std::vector<ASTNodePtr> a)
        : name(n), args(std::move(a)) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * ArrayAccessExpr — Array subscript: name[index]
 *------------------------------------------------------------------------*/
class ArrayAccessExpr : public ASTNode {
public:
    std::string name;
    ASTNodePtr  index;

    ArrayAccessExpr(const std::string& n, ASTNodePtr idx)
        : name(n), index(std::move(idx)) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * MemberAccessExpr — Object member access: object.member
 *------------------------------------------------------------------------*/
class MemberAccessExpr : public ASTNode {
public:
    ASTNodePtr  object;
    std::string member;

    MemberAccessExpr(ASTNodePtr obj, const std::string& mem)
        : object(std::move(obj)), member(mem) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * MethodCallExpr — Object method call: object.method(args)
 *------------------------------------------------------------------------*/
class MethodCallExpr : public ASTNode {
public:
    ASTNodePtr              object;
    std::string             method;
    std::vector<ASTNodePtr> args;

    MethodCallExpr(ASTNodePtr obj, const std::string& m, std::vector<ASTNodePtr> a)
        : object(std::move(obj)), method(m), args(std::move(a)) {}
    void accept(ASTVisitor& visitor) override;
};

/*==========================================================================
 * DECLARATION NODES — Variable, function, and class declarations
 *========================================================================*/

/*--------------------------------------------------------------------------
 * VarDeclaration — Variable declaration with optional initializer
 * e.g., "int x;" or "int x = 5;"
 *------------------------------------------------------------------------*/
class VarDeclaration : public ASTNode {
public:
    std::string typeName;   // "int", "float", "char", "bool", or class name
    std::string name;
    ASTNodePtr  initializer; // nullptr if no initializer

    VarDeclaration(const std::string& type, const std::string& name,
                   ASTNodePtr init = nullptr)
        : typeName(type), name(name), initializer(std::move(init)) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * ArrayDeclaration — Fixed-size array: "int arr[10];"
 *------------------------------------------------------------------------*/
class ArrayDeclaration : public ASTNode {
public:
    std::string typeName;
    std::string name;
    int         size;

    ArrayDeclaration(const std::string& type, const std::string& name, int sz)
        : typeName(type), name(name), size(sz) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * Parameter — Function parameter: "int x" or "int arr[]"
 *------------------------------------------------------------------------*/
class Parameter : public ASTNode {
public:
    std::string typeName;
    std::string name;
    bool        isArray;

    Parameter(const std::string& type, const std::string& name, bool arr = false)
        : typeName(type), name(name), isArray(arr) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * FunDeclaration — Function definition: "int foo(int a, float b) { ... }"
 *------------------------------------------------------------------------*/
class FunDeclaration : public ASTNode {
public:
    std::string                         returnType;
    std::string                         name;
    std::vector<std::unique_ptr<Parameter>> params;
    ASTNodePtr                          body;   // CompoundStmt

    FunDeclaration(const std::string& retType, const std::string& name,
                   std::vector<std::unique_ptr<Parameter>> params, ASTNodePtr body)
        : returnType(retType), name(name),
          params(std::move(params)), body(std::move(body)) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * ClassDeclaration — Class definition with members
 * e.g., "class Foo { public: int x; void bar(); private: int y; };"
 *------------------------------------------------------------------------*/
class ClassDeclaration : public ASTNode {
public:
    std::string             name;
    // Each member has an access level and a declaration node
    struct Member {
        std::string access;   // "public" or "private"
        ASTNodePtr  decl;     // VarDeclaration or FunDeclaration
    };
    std::vector<Member> members;

    ClassDeclaration(const std::string& name, std::vector<Member> members)
        : name(name), members(std::move(members)) {}
    void accept(ASTVisitor& visitor) override;
};

/*==========================================================================
 * STATEMENT NODES — Executable statements
 *========================================================================*/

/*--------------------------------------------------------------------------
 * ExpressionStmt — An expression used as a statement: "x + 1;" or "foo();"
 *------------------------------------------------------------------------*/
class ExpressionStmt : public ASTNode {
public:
    ASTNodePtr expr;
    explicit ExpressionStmt(ASTNodePtr e) : expr(std::move(e)) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * CompoundStmt — A block of statements: "{ stmt1; stmt2; ... }"
 * Also opens a new scope for the symbol table.
 *------------------------------------------------------------------------*/
class CompoundStmt : public ASTNode {
public:
    std::vector<ASTNodePtr> statements;
    explicit CompoundStmt(std::vector<ASTNodePtr> stmts)
        : statements(std::move(stmts)) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * IfStmt — Conditional: "if (cond) thenBranch [else elseBranch]"
 *------------------------------------------------------------------------*/
class IfStmt : public ASTNode {
public:
    ASTNodePtr condition;
    ASTNodePtr thenBranch;
    ASTNodePtr elseBranch;  // nullptr if no else

    IfStmt(ASTNodePtr cond, ASTNodePtr then, ASTNodePtr els = nullptr)
        : condition(std::move(cond)),
          thenBranch(std::move(then)),
          elseBranch(std::move(els)) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * WhileStmt — While loop: "while (cond) body"
 *------------------------------------------------------------------------*/
class WhileStmt : public ASTNode {
public:
    ASTNodePtr condition;
    ASTNodePtr body;

    WhileStmt(ASTNodePtr cond, ASTNodePtr body)
        : condition(std::move(cond)), body(std::move(body)) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * ForStmt — For loop: "for (init; cond; update) body"
 * COMPILER THEORY: A for loop is syntactic sugar that decomposes to:
 *   init; while(cond) { body; update; }
 *------------------------------------------------------------------------*/
class ForStmt : public ASTNode {
public:
    ASTNodePtr init;       // Initializer (declaration or expression)
    ASTNodePtr condition;
    ASTNodePtr update;     // Post-iteration expression
    ASTNodePtr body;

    ForStmt(ASTNodePtr init, ASTNodePtr cond, ASTNodePtr upd, ASTNodePtr body)
        : init(std::move(init)), condition(std::move(cond)),
          update(std::move(upd)), body(std::move(body)) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * ReturnStmt — Return from function: "return;" or "return expr;"
 *------------------------------------------------------------------------*/
class ReturnStmt : public ASTNode {
public:
    ASTNodePtr value;   // nullptr for "return;"
    explicit ReturnStmt(ASTNodePtr val = nullptr) : value(std::move(val)) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * CoutStmt — Output: "cout << expr1 << expr2 << endl;"
 * The expressions vector holds each item after <<.
 * Special: "endl" is represented as a StringLiteral("\\n").
 *------------------------------------------------------------------------*/
class CoutStmt : public ASTNode {
public:
    std::vector<ASTNodePtr> expressions;
    explicit CoutStmt(std::vector<ASTNodePtr> exprs)
        : expressions(std::move(exprs)) {}
    void accept(ASTVisitor& visitor) override;
};

/*--------------------------------------------------------------------------
 * CinStmt — Input: "cin >> var1 >> var2;"
 * The targets vector holds each variable after >>.
 *------------------------------------------------------------------------*/
class CinStmt : public ASTNode {
public:
    std::vector<ASTNodePtr> targets;
    explicit CinStmt(std::vector<ASTNodePtr> tgts)
        : targets(std::move(tgts)) {}
    void accept(ASTVisitor& visitor) override;
};

/*==========================================================================
 * PROGRAM NODE — The root of the entire AST
 *========================================================================*/

/*--------------------------------------------------------------------------
 * Program — Top-level node containing all declarations and statements.
 * The AST produced by the parser is a single Program node.
 *------------------------------------------------------------------------*/
class Program : public ASTNode {
public:
    std::vector<ASTNodePtr> declarations;
    explicit Program(std::vector<ASTNodePtr> decls)
        : declarations(std::move(decls)) {}
    void accept(ASTVisitor& visitor) override;
};

} // namespace seedscan

#endif // SEEDSCAN_AST_HPP
