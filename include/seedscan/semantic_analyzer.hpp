/***************************************************************************
 * semantic_analyzer.hpp — Type Checking & Scope Resolution
 * -------------------------------------------------------------------------
 * COMPILER THEORY — Semantic Analysis (Phase 3):
 * After parsing produces a syntactically valid AST, semantic analysis
 * checks for MEANING correctness that can't be expressed in the grammar:
 *   1. Variables must be declared before use
 *   2. Types must be compatible in expressions (int + float → float)
 *   3. Function calls must match parameter count and types
 *   4. Return types must match function signatures
 *   5. Array indices must be integers
 *   6. Class member access must respect public/private
 ***************************************************************************/
#ifndef SEEDSCAN_SEMANTIC_ANALYZER_HPP
#define SEEDSCAN_SEMANTIC_ANALYZER_HPP

#include "seedscan/ast_visitor.hpp"
#include "seedscan/symbol_table.hpp"
#include "seedscan/error_handler.hpp"
#include <string>

namespace seedscan {

class SemanticAnalyzer : public ASTVisitor {
public:
    SemanticAnalyzer(ErrorHandler& errorHandler);

    // Run analysis on the entire program
    void analyze(Program& program);

    // Get the inferred type of the last visited expression
    std::string lastExprType() const { return currentExprType_; }

    // Visitor methods
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
    SymbolTable   symTable_;
    ErrorHandler& errors_;
    std::string   currentExprType_;     // Type of the last evaluated expression
    std::string   currentFunctionReturnType_; // For return statement checking
    std::string   currentClassName_;     // For member access checking

    // Type compatibility check with implicit promotion
    std::string resolveType(const std::string& left, const std::string& right,
                            const std::string& op, int line, int col);

    // Check if a type is a valid base type
    bool isValidType(const std::string& type);

    // Check if assignment is compatible
    bool isAssignmentCompatible(const std::string& target, const std::string& value);
};

} // namespace seedscan

#endif // SEEDSCAN_SEMANTIC_ANALYZER_HPP
