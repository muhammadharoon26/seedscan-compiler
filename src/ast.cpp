/***************************************************************************
 * ast.cpp — AST Node accept() Method Implementations
 * -------------------------------------------------------------------------
 * Each accept() method implements the Visitor pattern's double dispatch.
 * When a visitor calls node->accept(visitor), the node calls back
 * visitor.visit(*this) with its concrete type, routing to the correct
 * overload in the visitor.
 ***************************************************************************/
#include "seedscan/ast.hpp"
#include "seedscan/ast_visitor.hpp"

/* Global AST root — set by the parser */
seedscan::Program* g_program = nullptr;

namespace seedscan {

// ── Program ───────────────────────────────────────────────────────────────
void Program::accept(ASTVisitor& v)          { v.visit(*this); }

// ── Declarations ──────────────────────────────────────────────────────────
void VarDeclaration::accept(ASTVisitor& v)   { v.visit(*this); }
void ArrayDeclaration::accept(ASTVisitor& v) { v.visit(*this); }
void FunDeclaration::accept(ASTVisitor& v)   { v.visit(*this); }
void ClassDeclaration::accept(ASTVisitor& v) { v.visit(*this); }
void Parameter::accept(ASTVisitor& v)        { v.visit(*this); }

// ── Statements ────────────────────────────────────────────────────────────
void ExpressionStmt::accept(ASTVisitor& v)   { v.visit(*this); }
void CompoundStmt::accept(ASTVisitor& v)     { v.visit(*this); }
void IfStmt::accept(ASTVisitor& v)           { v.visit(*this); }
void WhileStmt::accept(ASTVisitor& v)        { v.visit(*this); }
void ForStmt::accept(ASTVisitor& v)          { v.visit(*this); }
void ReturnStmt::accept(ASTVisitor& v)       { v.visit(*this); }
void CoutStmt::accept(ASTVisitor& v)         { v.visit(*this); }
void CinStmt::accept(ASTVisitor& v)          { v.visit(*this); }

// ── Expressions ───────────────────────────────────────────────────────────
void BinaryExpr::accept(ASTVisitor& v)       { v.visit(*this); }
void UnaryExpr::accept(ASTVisitor& v)        { v.visit(*this); }
void PostfixExpr::accept(ASTVisitor& v)      { v.visit(*this); }
void AssignExpr::accept(ASTVisitor& v)       { v.visit(*this); }
void CallExpr::accept(ASTVisitor& v)         { v.visit(*this); }
void ArrayAccessExpr::accept(ASTVisitor& v)  { v.visit(*this); }
void MemberAccessExpr::accept(ASTVisitor& v) { v.visit(*this); }
void MethodCallExpr::accept(ASTVisitor& v)   { v.visit(*this); }

// ── Literals & Identifiers ────────────────────────────────────────────────
void IdentifierExpr::accept(ASTVisitor& v)   { v.visit(*this); }
void IntLiteral::accept(ASTVisitor& v)       { v.visit(*this); }
void FloatLiteral::accept(ASTVisitor& v)     { v.visit(*this); }
void CharLiteral::accept(ASTVisitor& v)      { v.visit(*this); }
void StringLiteral::accept(ASTVisitor& v)    { v.visit(*this); }
void BoolLiteral::accept(ASTVisitor& v)      { v.visit(*this); }

} // namespace seedscan
