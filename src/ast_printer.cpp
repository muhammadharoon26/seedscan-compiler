/***************************************************************************
 * ast_printer.cpp — AST Pretty-Printer Implementation
 * -------------------------------------------------------------------------
 * Prints the AST as an indented tree. Example output:
 *   Program
 *     FunDeclaration: int main
 *       CompoundStmt
 *         VarDeclaration: int x = ...
 *           IntLiteral: 42
 ***************************************************************************/
#include "seedscan/ast_printer.hpp"
#include "seedscan/ast.hpp"

namespace seedscan {

void ASTPrinter::printIndent() {
    for (int i = 0; i < indent_; i++) {
        out_ << "  ";
    }
}

void ASTPrinter::visitChild(ASTNode* node) {
    if (node) {
        indent_++;
        node->accept(*this);
        indent_--;
    }
}

// ── Program ───────────────────────────────────────────────────────────────
void ASTPrinter::visit(Program& node) {
    printIndent(); out_ << "Program\n";
    for (auto& decl : node.declarations) { visitChild(decl.get()); }
}

// ── Declarations ──────────────────────────────────────────────────────────
void ASTPrinter::visit(VarDeclaration& node) {
    printIndent();
    out_ << "VarDecl: " << node.typeName << " " << node.name;
    if (node.initializer) {
        out_ << " =\n";
        visitChild(node.initializer.get());
    } else {
        out_ << "\n";
    }
}

void ASTPrinter::visit(ArrayDeclaration& node) {
    printIndent();
    out_ << "ArrayDecl: " << node.typeName << " " << node.name
         << "[" << node.size << "]\n";
}

void ASTPrinter::visit(FunDeclaration& node) {
    printIndent();
    out_ << "FunDecl: " << node.returnType << " " << node.name << "(";
    for (size_t i = 0; i < node.params.size(); i++) {
        if (i > 0) out_ << ", ";
        out_ << node.params[i]->typeName << " " << node.params[i]->name;
        if (node.params[i]->isArray) out_ << "[]";
    }
    out_ << ")\n";
    visitChild(node.body.get());
}

void ASTPrinter::visit(ClassDeclaration& node) {
    printIndent();
    out_ << "ClassDecl: " << node.name << "\n";
    for (auto& member : node.members) {
        indent_++;
        printIndent();
        out_ << "[" << member.access << "]\n";
        visitChild(member.decl.get());
        indent_--;
    }
}

void ASTPrinter::visit(Parameter& node) {
    printIndent();
    out_ << "Param: " << node.typeName << " " << node.name;
    if (node.isArray) out_ << "[]";
    out_ << "\n";
}

// ── Statements ────────────────────────────────────────────────────────────
void ASTPrinter::visit(ExpressionStmt& node) {
    printIndent(); out_ << "ExprStmt\n";
    visitChild(node.expr.get());
}

void ASTPrinter::visit(CompoundStmt& node) {
    printIndent(); out_ << "Block\n";
    for (auto& stmt : node.statements) { visitChild(stmt.get()); }
}

void ASTPrinter::visit(IfStmt& node) {
    printIndent(); out_ << "IfStmt\n";
    indent_++;
    printIndent(); out_ << "Condition:\n"; visitChild(node.condition.get());
    printIndent(); out_ << "Then:\n"; visitChild(node.thenBranch.get());
    if (node.elseBranch) {
        printIndent(); out_ << "Else:\n"; visitChild(node.elseBranch.get());
    }
    indent_--;
}

void ASTPrinter::visit(WhileStmt& node) {
    printIndent(); out_ << "WhileStmt\n";
    indent_++;
    printIndent(); out_ << "Condition:\n"; visitChild(node.condition.get());
    printIndent(); out_ << "Body:\n"; visitChild(node.body.get());
    indent_--;
}

void ASTPrinter::visit(ForStmt& node) {
    printIndent(); out_ << "ForStmt\n";
    indent_++;
    printIndent(); out_ << "Init:\n"; visitChild(node.init.get());
    printIndent(); out_ << "Condition:\n"; visitChild(node.condition.get());
    printIndent(); out_ << "Update:\n"; visitChild(node.update.get());
    printIndent(); out_ << "Body:\n"; visitChild(node.body.get());
    indent_--;
}

void ASTPrinter::visit(ReturnStmt& node) {
    printIndent(); out_ << "ReturnStmt\n";
    if (node.value) { visitChild(node.value.get()); }
}

void ASTPrinter::visit(CoutStmt& node) {
    printIndent(); out_ << "CoutStmt\n";
    for (auto& expr : node.expressions) { visitChild(expr.get()); }
}

void ASTPrinter::visit(CinStmt& node) {
    printIndent(); out_ << "CinStmt\n";
    for (auto& tgt : node.targets) { visitChild(tgt.get()); }
}

// ── Expressions ───────────────────────────────────────────────────────────
void ASTPrinter::visit(BinaryExpr& node) {
    printIndent(); out_ << "BinaryExpr: " << node.op << "\n";
    visitChild(node.left.get());
    visitChild(node.right.get());
}

void ASTPrinter::visit(UnaryExpr& node) {
    printIndent(); out_ << "UnaryExpr: " << node.op << "\n";
    visitChild(node.operand.get());
}

void ASTPrinter::visit(PostfixExpr& node) {
    printIndent(); out_ << "PostfixExpr: " << node.op << "\n";
    visitChild(node.operand.get());
}

void ASTPrinter::visit(AssignExpr& node) {
    printIndent(); out_ << "AssignExpr: " << node.op << "\n";
    visitChild(node.target.get());
    visitChild(node.value.get());
}

void ASTPrinter::visit(CallExpr& node) {
    printIndent(); out_ << "CallExpr: " << node.name << "\n";
    for (auto& arg : node.args) { visitChild(arg.get()); }
}

void ASTPrinter::visit(ArrayAccessExpr& node) {
    printIndent(); out_ << "ArrayAccess: " << node.name << "[]\n";
    visitChild(node.index.get());
}

void ASTPrinter::visit(MemberAccessExpr& node) {
    printIndent(); out_ << "MemberAccess: ." << node.member << "\n";
    visitChild(node.object.get());
}

void ASTPrinter::visit(MethodCallExpr& node) {
    printIndent(); out_ << "MethodCall: ." << node.method << "()\n";
    visitChild(node.object.get());
    for (auto& arg : node.args) { visitChild(arg.get()); }
}

// ── Literals & Identifiers ────────────────────────────────────────────────
void ASTPrinter::visit(IdentifierExpr& node) {
    printIndent(); out_ << "Identifier: " << node.name << "\n";
}

void ASTPrinter::visit(IntLiteral& node) {
    printIndent(); out_ << "IntLiteral: " << node.value << "\n";
}

void ASTPrinter::visit(FloatLiteral& node) {
    printIndent(); out_ << "FloatLiteral: " << node.value << "\n";
}

void ASTPrinter::visit(CharLiteral& node) {
    printIndent(); out_ << "CharLiteral: '" << node.value << "'\n";
}

void ASTPrinter::visit(StringLiteral& node) {
    printIndent(); out_ << "StringLiteral: \"" << node.value << "\"\n";
}

void ASTPrinter::visit(BoolLiteral& node) {
    printIndent(); out_ << "BoolLiteral: " << (node.value ? "true" : "false") << "\n";
}

} // namespace seedscan
