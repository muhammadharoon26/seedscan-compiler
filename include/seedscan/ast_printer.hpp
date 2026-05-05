/***************************************************************************
 * ast_printer.hpp — AST Pretty-Printer (Visitor Implementation)
 * -------------------------------------------------------------------------
 * Traverses the AST and prints a human-readable, indented tree
 * representation. Used for the --ast CLI flag and debugging.
 ***************************************************************************/
#ifndef SEEDSCAN_AST_PRINTER_HPP
#define SEEDSCAN_AST_PRINTER_HPP

#include "seedscan/ast_visitor.hpp"
#include <iostream>
#include <string>

namespace seedscan {

class ASTNode;

class ASTPrinter : public ASTVisitor {
public:
    explicit ASTPrinter(std::ostream& out = std::cout) : out_(out), indent_(0) {}

    void visit(Program& node) override;
    void visit(VarDeclaration& node) override;
    void visit(ArrayDeclaration& node) override;
    void visit(FunDeclaration& node) override;
    void visit(ClassDeclaration& node) override;
    void visit(Parameter& node) override;
    void visit(ExpressionStmt& node) override;
    void visit(CompoundStmt& node) override;
    void visit(IfStmt& node) override;
    void visit(WhileStmt& node) override;
    void visit(ForStmt& node) override;
    void visit(ReturnStmt& node) override;
    void visit(CoutStmt& node) override;
    void visit(CinStmt& node) override;
    void visit(BinaryExpr& node) override;
    void visit(UnaryExpr& node) override;
    void visit(PostfixExpr& node) override;
    void visit(AssignExpr& node) override;
    void visit(CallExpr& node) override;
    void visit(ArrayAccessExpr& node) override;
    void visit(MemberAccessExpr& node) override;
    void visit(MethodCallExpr& node) override;
    void visit(IdentifierExpr& node) override;
    void visit(IntLiteral& node) override;
    void visit(FloatLiteral& node) override;
    void visit(CharLiteral& node) override;
    void visit(StringLiteral& node) override;
    void visit(BoolLiteral& node) override;

private:
    std::ostream& out_;
    int indent_;

    // Print indentation prefix (2 spaces per level, with tree-drawing chars)
    void printIndent();
    // Increase indent, visit child, decrease indent
    void visitChild(ASTNode* node);
};

} // namespace seedscan

#endif // SEEDSCAN_AST_PRINTER_HPP
