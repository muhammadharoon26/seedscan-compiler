/***************************************************************************
 * ast_visitor.hpp — Visitor Pattern Interface for AST Traversal
 * -------------------------------------------------------------------------
 * COMPILER THEORY — Visitor Pattern:
 * The Visitor pattern separates algorithms from the object structure they
 * operate on. In a compiler, the AST is the structure, and the visitors
 * are the different compiler passes:
 *   1. ASTPrinter    — Pretty-prints the tree (debugging)
 *   2. SemanticAnalyzer — Type checking and scope resolution
 *   3. IRGenerator   — Three-address code generation
 *
 * Each visitor implements visit() for every node type. Each AST node
 * implements accept(visitor) which calls visitor.visit(*this).
 * This "double dispatch" lets us add new passes without touching nodes.
 ***************************************************************************/
#ifndef SEEDSCAN_AST_VISITOR_HPP
#define SEEDSCAN_AST_VISITOR_HPP

namespace seedscan {

// Forward-declare all AST node types
class Program;
class VarDeclaration;
class ArrayDeclaration;
class FunDeclaration;
class ClassDeclaration;
class Parameter;
class ExpressionStmt;
class CompoundStmt;
class IfStmt;
class WhileStmt;
class ForStmt;
class ReturnStmt;
class CoutStmt;
class CinStmt;
class BinaryExpr;
class UnaryExpr;
class PostfixExpr;
class AssignExpr;
class CallExpr;
class ArrayAccessExpr;
class MemberAccessExpr;
class MethodCallExpr;
class IdentifierExpr;
class IntLiteral;
class FloatLiteral;
class CharLiteral;
class StringLiteral;
class BoolLiteral;

/*--------------------------------------------------------------------------
 * ASTVisitor — Abstract base class for all AST visitors.
 *
 * Every concrete visitor (printer, semantic analyzer, IR generator) must
 * override all visit() methods. This enforces exhaustive handling of every
 * node type at compile time.
 *------------------------------------------------------------------------*/
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    // Program root
    virtual void visit(Program& node) = 0;

    // Declarations
    virtual void visit(VarDeclaration& node) = 0;
    virtual void visit(ArrayDeclaration& node) = 0;
    virtual void visit(FunDeclaration& node) = 0;
    virtual void visit(ClassDeclaration& node) = 0;
    virtual void visit(Parameter& node) = 0;

    // Statements
    virtual void visit(ExpressionStmt& node) = 0;
    virtual void visit(CompoundStmt& node) = 0;
    virtual void visit(IfStmt& node) = 0;
    virtual void visit(WhileStmt& node) = 0;
    virtual void visit(ForStmt& node) = 0;
    virtual void visit(ReturnStmt& node) = 0;
    virtual void visit(CoutStmt& node) = 0;
    virtual void visit(CinStmt& node) = 0;

    // Expressions
    virtual void visit(BinaryExpr& node) = 0;
    virtual void visit(UnaryExpr& node) = 0;
    virtual void visit(PostfixExpr& node) = 0;
    virtual void visit(AssignExpr& node) = 0;
    virtual void visit(CallExpr& node) = 0;
    virtual void visit(ArrayAccessExpr& node) = 0;
    virtual void visit(MemberAccessExpr& node) = 0;
    virtual void visit(MethodCallExpr& node) = 0;

    // Literals and identifiers
    virtual void visit(IdentifierExpr& node) = 0;
    virtual void visit(IntLiteral& node) = 0;
    virtual void visit(FloatLiteral& node) = 0;
    virtual void visit(CharLiteral& node) = 0;
    virtual void visit(StringLiteral& node) = 0;
    virtual void visit(BoolLiteral& node) = 0;
};

} // namespace seedscan

#endif // SEEDSCAN_AST_VISITOR_HPP
