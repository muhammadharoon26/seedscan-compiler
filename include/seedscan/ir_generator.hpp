/***************************************************************************
 * ir_generator.hpp — TAC IR Generator (Visitor Implementation)
 * -------------------------------------------------------------------------
 * COMPILER THEORY — Code Generation:
 * The IR generator is the FINAL phase in our pipeline. It walks the
 * semantically-verified AST and emits Three-Address Code instructions.
 * Each expression evaluates to a temporary, and control flow uses
 * labels and conditional/unconditional jumps.
 ***************************************************************************/
#ifndef SEEDSCAN_IR_GENERATOR_HPP
#define SEEDSCAN_IR_GENERATOR_HPP

#include "seedscan/ast_visitor.hpp"
#include "seedscan/tac.hpp"
#include <string>

namespace seedscan {

class IRGenerator : public ASTVisitor {
public:
    IRGenerator();

    // Generate IR for the entire program
    void generate(Program& program);

    // Get the generated TAC program
    const TACProgram& getProgram() const { return program_; }
    TACProgram& getProgram() { return program_; }

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
    TACProgram  program_;
    std::string lastResult_;  // Temp/var holding the last expression result
};

} // namespace seedscan

#endif // SEEDSCAN_IR_GENERATOR_HPP
