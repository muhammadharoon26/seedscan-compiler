/***************************************************************************
 * ir_generator.cpp — Three-Address Code Generation Implementation
 * -------------------------------------------------------------------------
 * COMPILER THEORY — TAC Generation:
 * Each AST node maps to one or more TAC instructions:
 *   - Expressions → temp = arg1 OP arg2
 *   - If/else    → conditional jump + labels
 *   - While/For  → labels + backward jump
 *   - Functions  → PARAM + CALL instructions
 *   - I/O        → WRITE / READ instructions
 ***************************************************************************/
#include "seedscan/ir_generator.hpp"
#include "seedscan/ast.hpp"
#include <sstream>

namespace seedscan {

// ── TACInstruction::toString ──────────────────────────────────────────────
std::string TACInstruction::toString() const {
    std::ostringstream ss;
    switch (op) {
        case TACOp::ADD:  ss << result << " = " << arg1 << " + " << arg2; break;
        case TACOp::SUB:  ss << result << " = " << arg1 << " - " << arg2; break;
        case TACOp::MUL:  ss << result << " = " << arg1 << " * " << arg2; break;
        case TACOp::DIV:  ss << result << " = " << arg1 << " / " << arg2; break;
        case TACOp::MOD:  ss << result << " = " << arg1 << " % " << arg2; break;
        case TACOp::NEG:  ss << result << " = -" << arg1; break;
        case TACOp::EQ:   ss << result << " = " << arg1 << " == " << arg2; break;
        case TACOp::NEQ:  ss << result << " = " << arg1 << " != " << arg2; break;
        case TACOp::LT:   ss << result << " = " << arg1 << " < "  << arg2; break;
        case TACOp::GT:   ss << result << " = " << arg1 << " > "  << arg2; break;
        case TACOp::LTE:  ss << result << " = " << arg1 << " <= " << arg2; break;
        case TACOp::GTE:  ss << result << " = " << arg1 << " >= " << arg2; break;
        case TACOp::AND:  ss << result << " = " << arg1 << " && " << arg2; break;
        case TACOp::OR:   ss << result << " = " << arg1 << " || " << arg2; break;
        case TACOp::NOT:  ss << result << " = !" << arg1; break;
        case TACOp::ASSIGN: ss << result << " = " << arg1; break;
        case TACOp::LABEL:  ss << result << ":"; break;
        case TACOp::GOTO:   ss << "goto " << result; break;
        case TACOp::IF_GOTO:       ss << "if " << arg1 << " goto " << result; break;
        case TACOp::IF_FALSE_GOTO: ss << "if_false " << arg1 << " goto " << result; break;
        case TACOp::PARAM:  ss << "param " << result; break;
        case TACOp::CALL:   ss << result << " = call " << arg1 << ", " << arg2; break;
        case TACOp::RETURN: ss << "return" << (result.empty() ? "" : " " + result); break;
        case TACOp::WRITE:  ss << "write " << result; break;
        case TACOp::WRITELN: ss << "writeln"; break;
        case TACOp::READ:   ss << "read " << result; break;
        case TACOp::ARRAY_LOAD:  ss << result << " = " << arg1 << "[" << arg2 << "]"; break;
        case TACOp::ARRAY_STORE: ss << result << "[" << arg1 << "] = " << arg2; break;
        case TACOp::MEMBER_LOAD:  ss << result << " = " << arg1 << "." << arg2; break;
        case TACOp::MEMBER_STORE: ss << result << "." << arg1 << " = " << arg2; break;
        case TACOp::INC:  ss << result << " = " << arg1 << " + 1"; break;
        case TACOp::DEC:  ss << result << " = " << arg1 << " - 1"; break;
    }
    return ss.str();
}

// ── TACProgram methods ────────────────────────────────────────────────────
void TACProgram::emit(const TACInstruction& instr) {
    instructions_.push_back(instr);
}

void TACProgram::emit(TACOp op, const std::string& result,
                      const std::string& arg1, const std::string& arg2) {
    instructions_.emplace_back(op, result, arg1, arg2);
}

std::string TACProgram::newTemp() {
    return "t" + std::to_string(tempCounter_++);
}

std::string TACProgram::newLabel() {
    return "L" + std::to_string(labelCounter_++);
}

std::string TACProgram::toString() const {
    std::ostringstream ss;
    for (const auto& instr : instructions_) {
        // Labels are not indented; instructions are indented
        if (instr.op == TACOp::LABEL) {
            ss << instr.toString() << "\n";
        } else {
            ss << "    " << instr.toString() << "\n";
        }
    }
    return ss.str();
}

// ── IRGenerator ───────────────────────────────────────────────────────────
IRGenerator::IRGenerator() {}

void IRGenerator::generate(Program& program) {
    program.accept(*this);
}

// Map string operators to TACOp
static TACOp opFromString(const std::string& op) {
    if (op == "+")  return TACOp::ADD;
    if (op == "-")  return TACOp::SUB;
    if (op == "*")  return TACOp::MUL;
    if (op == "/")  return TACOp::DIV;
    if (op == "%")  return TACOp::MOD;
    if (op == "==") return TACOp::EQ;
    if (op == "!=") return TACOp::NEQ;
    if (op == "<")  return TACOp::LT;
    if (op == ">")  return TACOp::GT;
    if (op == "<=") return TACOp::LTE;
    if (op == ">=") return TACOp::GTE;
    if (op == "&&") return TACOp::AND;
    if (op == "||") return TACOp::OR;
    return TACOp::ADD; // fallback
}

// ── Program ───────────────────────────────────────────────────────────────
void IRGenerator::visit(Program& node) {
    for (auto& decl : node.declarations) {
        if (decl) decl->accept(*this);
    }
}

// ── Declarations ──────────────────────────────────────────────────────────
void IRGenerator::visit(VarDeclaration& node) {
    if (node.initializer) {
        node.initializer->accept(*this);
        program_.emit(TACOp::ASSIGN, node.name, lastResult_);
    }
}

void IRGenerator::visit(ArrayDeclaration&) {
    // Array declarations don't generate TAC — space is allocated by the runtime
}

void IRGenerator::visit(FunDeclaration& node) {
    // Emit function label
    program_.emit(TACOp::LABEL, "func_" + node.name);
    if (node.body) node.body->accept(*this);
    // Implicit return for void functions
    if (node.returnType == "void") {
        program_.emit(TACOp::RETURN, "");
    }
}

void IRGenerator::visit(ClassDeclaration& node) {
    // Emit class methods as labeled functions
    for (auto& member : node.members) {
        if (auto* fd = dynamic_cast<FunDeclaration*>(member.decl.get())) {
            program_.emit(TACOp::LABEL, "func_" + node.name + "_" + fd->name);
            if (fd->body) fd->body->accept(*this);
            if (fd->returnType == "void") {
                program_.emit(TACOp::RETURN, "");
            }
        }
    }
}

void IRGenerator::visit(Parameter&) {
    // Parameters are handled by the function call mechanism
}

// ── Statements ────────────────────────────────────────────────────────────
void IRGenerator::visit(ExpressionStmt& node) {
    if (node.expr) node.expr->accept(*this);
}

void IRGenerator::visit(CompoundStmt& node) {
    for (auto& stmt : node.statements) {
        if (stmt) stmt->accept(*this);
    }
}

/*--------------------------------------------------------------------------
 * IfStmt TAC Pattern:
 *   <condition code>
 *   if_false condResult goto L_else
 *   <then code>
 *   goto L_end
 *   L_else:
 *   <else code>
 *   L_end:
 *------------------------------------------------------------------------*/
void IRGenerator::visit(IfStmt& node) {
    node.condition->accept(*this);
    std::string condResult = lastResult_;

    std::string labelElse = program_.newLabel();
    std::string labelEnd  = program_.newLabel();

    program_.emit(TACOp::IF_FALSE_GOTO, labelElse, condResult);
    node.thenBranch->accept(*this);

    if (node.elseBranch) {
        program_.emit(TACOp::GOTO, labelEnd);
        program_.emit(TACOp::LABEL, labelElse);
        node.elseBranch->accept(*this);
        program_.emit(TACOp::LABEL, labelEnd);
    } else {
        program_.emit(TACOp::LABEL, labelElse);
    }
}

/*--------------------------------------------------------------------------
 * WhileStmt TAC Pattern:
 *   L_start:
 *   <condition code>
 *   if_false condResult goto L_end
 *   <body code>
 *   goto L_start
 *   L_end:
 *------------------------------------------------------------------------*/
void IRGenerator::visit(WhileStmt& node) {
    std::string labelStart = program_.newLabel();
    std::string labelEnd   = program_.newLabel();

    program_.emit(TACOp::LABEL, labelStart);
    node.condition->accept(*this);
    program_.emit(TACOp::IF_FALSE_GOTO, labelEnd, lastResult_);
    node.body->accept(*this);
    program_.emit(TACOp::GOTO, labelStart);
    program_.emit(TACOp::LABEL, labelEnd);
}

/*--------------------------------------------------------------------------
 * ForStmt TAC Pattern (decomposed to while):
 *   <init code>
 *   L_start:
 *   <condition code>
 *   if_false condResult goto L_end
 *   <body code>
 *   <update code>
 *   goto L_start
 *   L_end:
 *------------------------------------------------------------------------*/
void IRGenerator::visit(ForStmt& node) {
    if (node.init) node.init->accept(*this);

    std::string labelStart = program_.newLabel();
    std::string labelEnd   = program_.newLabel();

    program_.emit(TACOp::LABEL, labelStart);
    if (node.condition) {
        node.condition->accept(*this);
        program_.emit(TACOp::IF_FALSE_GOTO, labelEnd, lastResult_);
    }
    if (node.body) node.body->accept(*this);
    if (node.update) node.update->accept(*this);
    program_.emit(TACOp::GOTO, labelStart);
    program_.emit(TACOp::LABEL, labelEnd);
}

void IRGenerator::visit(ReturnStmt& node) {
    if (node.value) {
        node.value->accept(*this);
        program_.emit(TACOp::RETURN, lastResult_);
    } else {
        program_.emit(TACOp::RETURN, "");
    }
}

void IRGenerator::visit(CoutStmt& node) {
    for (auto& expr : node.expressions) {
        expr->accept(*this);
        if (auto* sl = dynamic_cast<StringLiteral*>(expr.get())) {
            if (sl->value == "\\n") {
                program_.emit(TACOp::WRITELN, "");
                continue;
            }
        }
        program_.emit(TACOp::WRITE, lastResult_);
    }
}

void IRGenerator::visit(CinStmt& node) {
    for (auto& target : node.targets) {
        target->accept(*this);
        program_.emit(TACOp::READ, lastResult_);
    }
}

// ── Expressions ───────────────────────────────────────────────────────────
void IRGenerator::visit(BinaryExpr& node) {
    node.left->accept(*this);
    std::string leftTemp = lastResult_;
    node.right->accept(*this);
    std::string rightTemp = lastResult_;

    std::string result = program_.newTemp();
    program_.emit(opFromString(node.op), result, leftTemp, rightTemp);
    lastResult_ = result;
}

void IRGenerator::visit(UnaryExpr& node) {
    node.operand->accept(*this);
    std::string operandTemp = lastResult_;
    std::string result = program_.newTemp();

    if (node.op == "-") {
        program_.emit(TACOp::NEG, result, operandTemp);
    } else if (node.op == "!") {
        program_.emit(TACOp::NOT, result, operandTemp);
    } else if (node.op == "++") {
        program_.emit(TACOp::INC, result, operandTemp);
        program_.emit(TACOp::ASSIGN, operandTemp, result);
    } else if (node.op == "--") {
        program_.emit(TACOp::DEC, result, operandTemp);
        program_.emit(TACOp::ASSIGN, operandTemp, result);
    }
    lastResult_ = result;
}

void IRGenerator::visit(PostfixExpr& node) {
    node.operand->accept(*this);
    std::string operandTemp = lastResult_;
    std::string oldValue = program_.newTemp();
    std::string newValue = program_.newTemp();

    // Save old value (postfix returns the OLD value)
    program_.emit(TACOp::ASSIGN, oldValue, operandTemp);
    if (node.op == "++") {
        program_.emit(TACOp::INC, newValue, operandTemp);
    } else {
        program_.emit(TACOp::DEC, newValue, operandTemp);
    }
    program_.emit(TACOp::ASSIGN, operandTemp, newValue);
    lastResult_ = oldValue;
}

void IRGenerator::visit(AssignExpr& node) {
    node.value->accept(*this);
    std::string valueTemp = lastResult_;

    // Determine the target name
    std::string targetName;
    if (auto* id = dynamic_cast<IdentifierExpr*>(node.target.get())) {
        targetName = id->name;
    } else if (auto* arr = dynamic_cast<ArrayAccessExpr*>(node.target.get())) {
        arr->index->accept(*this);
        std::string indexTemp = lastResult_;
        if (node.op != "=") {
            // Compound assignment: load, compute, store
            std::string loaded = program_.newTemp();
            program_.emit(TACOp::ARRAY_LOAD, loaded, arr->name, indexTemp);
            std::string result = program_.newTemp();
            std::string simpleOp = node.op.substr(0, node.op.size()-1); // "+=" → "+"
            program_.emit(opFromString(simpleOp), result, loaded, valueTemp);
            program_.emit(TACOp::ARRAY_STORE, arr->name, indexTemp, result);
            lastResult_ = result;
        } else {
            program_.emit(TACOp::ARRAY_STORE, arr->name, indexTemp, valueTemp);
            lastResult_ = valueTemp;
        }
        return;
    }

    if (node.op == "=") {
        program_.emit(TACOp::ASSIGN, targetName, valueTemp);
    } else {
        // Compound assignment: x += val → t = x + val; x = t
        std::string result = program_.newTemp();
        std::string simpleOp = node.op.substr(0, node.op.size()-1);
        program_.emit(opFromString(simpleOp), result, targetName, valueTemp);
        program_.emit(TACOp::ASSIGN, targetName, result);
        lastResult_ = result;
        return;
    }
    lastResult_ = targetName;
}

void IRGenerator::visit(CallExpr& node) {
    // Emit PARAM instructions for each argument
    for (auto& arg : node.args) {
        arg->accept(*this);
        program_.emit(TACOp::PARAM, lastResult_);
    }
    std::string result = program_.newTemp();
    program_.emit(TACOp::CALL, result, node.name, std::to_string(node.args.size()));
    lastResult_ = result;
}

void IRGenerator::visit(ArrayAccessExpr& node) {
    node.index->accept(*this);
    std::string indexTemp = lastResult_;
    std::string result = program_.newTemp();
    program_.emit(TACOp::ARRAY_LOAD, result, node.name, indexTemp);
    lastResult_ = result;
}

void IRGenerator::visit(MemberAccessExpr& node) {
    node.object->accept(*this);
    std::string objTemp = lastResult_;
    std::string result = program_.newTemp();
    program_.emit(TACOp::MEMBER_LOAD, result, objTemp, node.member);
    lastResult_ = result;
}

void IRGenerator::visit(MethodCallExpr& node) {
    node.object->accept(*this);
    std::string objTemp = lastResult_;
    program_.emit(TACOp::PARAM, objTemp); // 'this' pointer
    for (auto& arg : node.args) {
        arg->accept(*this);
        program_.emit(TACOp::PARAM, lastResult_);
    }
    std::string result = program_.newTemp();
    program_.emit(TACOp::CALL, result, node.method, std::to_string(node.args.size() + 1));
    lastResult_ = result;
}

// ── Literals & Identifiers ────────────────────────────────────────────────
void IRGenerator::visit(IdentifierExpr& node)  { lastResult_ = node.name; }
void IRGenerator::visit(IntLiteral& node)      { lastResult_ = std::to_string(node.value); }
void IRGenerator::visit(FloatLiteral& node)    { lastResult_ = std::to_string(node.value); }
void IRGenerator::visit(CharLiteral& node)     { lastResult_ = "'" + std::string(1, node.value) + "'"; }
void IRGenerator::visit(StringLiteral& node)   { lastResult_ = "\"" + node.value + "\""; }
void IRGenerator::visit(BoolLiteral& node)     { lastResult_ = node.value ? "1" : "0"; }

} // namespace seedscan
