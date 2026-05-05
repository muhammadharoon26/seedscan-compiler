/***************************************************************************
 * semantic_analyzer.cpp — Semantic Analysis Implementation
 * -------------------------------------------------------------------------
 * Walks the AST to enforce type safety, scope rules, and language semantics.
 ***************************************************************************/
#include "seedscan/semantic_analyzer.hpp"
#include "seedscan/ast.hpp"

namespace seedscan {

SemanticAnalyzer::SemanticAnalyzer(ErrorHandler& errorHandler)
    : errors_(errorHandler), currentExprType_("void") {}

void SemanticAnalyzer::analyze(Program& program) {
    program.accept(*this);
}

bool SemanticAnalyzer::isValidType(const std::string& type) {
    return type == "int" || type == "float" || type == "char" ||
           type == "bool" || type == "void" || type == "string" ||
           symTable_.lookup(type).has_value(); // class types
}

bool SemanticAnalyzer::isAssignmentCompatible(const std::string& target,
                                               const std::string& value) {
    if (target == value) return true;
    // Implicit promotion: int -> float
    if (target == "float" && value == "int") return true;
    // int and bool are interchangeable (C-style)
    if ((target == "int" && value == "bool") ||
        (target == "bool" && value == "int")) return true;
    // int and char are interchangeable (ASCII)
    if ((target == "int" && value == "char") ||
        (target == "char" && value == "int")) return true;
    return false;
}

/*--------------------------------------------------------------------------
 * resolveType — Determines the result type of a binary operation.
 * COMPILER THEORY — Type Promotion:
 *   int   OP int   → int
 *   float OP float → float
 *   int   OP float → float  (implicit widening conversion)
 *   Comparison ops always produce bool.
 *------------------------------------------------------------------------*/
std::string SemanticAnalyzer::resolveType(const std::string& left,
                                           const std::string& right,
                                           const std::string& op,
                                           int line, int col) {
    // Comparison operators always return bool
    if (op == "==" || op == "!=" || op == "<" || op == ">" ||
        op == "<=" || op == ">=") {
        if (!isAssignmentCompatible(left, right) &&
            !isAssignmentCompatible(right, left)) {
            errors_.reportError("Cannot compare '" + left + "' with '" + right + "'",
                               line, col, "Semantic");
        }
        return "bool";
    }

    // Logical operators require bool/int operands, return bool
    if (op == "&&" || op == "||") {
        return "bool";
    }

    // Arithmetic operators
    if (left == "float" || right == "float") return "float";
    if (left == "int" && right == "int") return "int";
    if (left == "char" && right == "char") return "int"; // char arithmetic → int

    errors_.reportError("Invalid operand types '" + left + "' and '" + right +
                        "' for operator '" + op + "'", line, col, "Semantic");
    return "int"; // fallback
}

// ── Program ───────────────────────────────────────────────────────────────
void SemanticAnalyzer::visit(Program& node) {
    for (auto& decl : node.declarations) {
        if (decl) decl->accept(*this);
    }
}

// ── Declarations ──────────────────────────────────────────────────────────
void SemanticAnalyzer::visit(VarDeclaration& node) {
    if (!isValidType(node.typeName)) {
        errors_.reportError("Unknown type '" + node.typeName + "'",
                           node.line, node.column, "Semantic");
    }
    if (node.typeName == "void") {
        errors_.reportError("Cannot declare variable of type 'void'",
                           node.line, node.column, "Semantic");
    }

    Symbol sym(node.name, node.typeName, SymbolKind::VARIABLE,
               symTable_.currentScopeLevel(), node.line, node.column);
    if (!symTable_.declare(sym)) {
        errors_.reportError("Redeclaration of variable '" + node.name + "'",
                           node.line, node.column, "Semantic");
    }

    if (node.initializer) {
        node.initializer->accept(*this);
        if (!isAssignmentCompatible(node.typeName, currentExprType_)) {
            errors_.reportError("Cannot initialize '" + node.typeName + "' with '" +
                               currentExprType_ + "'", node.line, node.column, "Semantic");
        }
    }
}

void SemanticAnalyzer::visit(ArrayDeclaration& node) {
    if (node.size <= 0) {
        errors_.reportError("Array size must be positive",
                           node.line, node.column, "Semantic");
    }
    Symbol sym(node.name, node.typeName, SymbolKind::ARRAY,
               symTable_.currentScopeLevel(), node.line, node.column);
    sym.arraySize = node.size;
    if (!symTable_.declare(sym)) {
        errors_.reportError("Redeclaration of array '" + node.name + "'",
                           node.line, node.column, "Semantic");
    }
}

void SemanticAnalyzer::visit(FunDeclaration& node) {
    Symbol funSym(node.name, node.returnType, SymbolKind::FUNCTION,
                  symTable_.currentScopeLevel(), node.line, node.column);
    funSym.returnType = node.returnType;
    for (auto& p : node.params) {
        funSym.paramTypes.push_back(p->typeName);
    }
    if (!symTable_.declare(funSym)) {
        errors_.reportError("Redeclaration of function '" + node.name + "'",
                           node.line, node.column, "Semantic");
    }

    // Enter function scope and declare parameters
    std::string prevReturnType = currentFunctionReturnType_;
    currentFunctionReturnType_ = node.returnType;
    symTable_.enterScope();

    for (auto& p : node.params) {
        p->accept(*this);
    }

    if (node.body) node.body->accept(*this);

    symTable_.exitScope();
    currentFunctionReturnType_ = prevReturnType;
}

void SemanticAnalyzer::visit(ClassDeclaration& node) {
    Symbol classSym(node.name, node.name, SymbolKind::CLASS,
                    symTable_.currentScopeLevel(), node.line, node.column);
    if (!symTable_.declare(classSym)) {
        errors_.reportError("Redeclaration of class '" + node.name + "'",
                           node.line, node.column, "Semantic");
    }

    std::string prevClassName = currentClassName_;
    currentClassName_ = node.name;
    symTable_.enterScope();

    for (auto& member : node.members) {
        if (member.decl) {
            member.decl->accept(*this);
            // Register member in class member table for access checking
            // Extract name from the declaration
            if (auto* vd = dynamic_cast<VarDeclaration*>(member.decl.get())) {
                Symbol memSym(vd->name, vd->typeName, SymbolKind::MEMBER_VAR,
                              symTable_.currentScopeLevel(), vd->line, vd->column);
                memSym.accessLevel = member.access;
                symTable_.declareClassMember(node.name, memSym);
            } else if (auto* fd = dynamic_cast<FunDeclaration*>(member.decl.get())) {
                Symbol memSym(fd->name, fd->returnType, SymbolKind::MEMBER_FUNC,
                              symTable_.currentScopeLevel(), fd->line, fd->column);
                memSym.accessLevel = member.access;
                for (auto& p : fd->params) memSym.paramTypes.push_back(p->typeName);
                symTable_.declareClassMember(node.name, memSym);
            }
        }
    }

    symTable_.exitScope();
    currentClassName_ = prevClassName;
}

void SemanticAnalyzer::visit(Parameter& node) {
    Symbol sym(node.name, node.typeName, SymbolKind::PARAMETER,
               symTable_.currentScopeLevel(), node.line, node.column);
    if (!symTable_.declare(sym)) {
        errors_.reportError("Duplicate parameter name '" + node.name + "'",
                           node.line, node.column, "Semantic");
    }
}

// ── Statements ────────────────────────────────────────────────────────────
void SemanticAnalyzer::visit(ExpressionStmt& node) {
    if (node.expr) node.expr->accept(*this);
}

void SemanticAnalyzer::visit(CompoundStmt& node) {
    symTable_.enterScope();
    for (auto& stmt : node.statements) {
        if (stmt) stmt->accept(*this);
    }
    symTable_.exitScope();
}

void SemanticAnalyzer::visit(IfStmt& node) {
    node.condition->accept(*this);
    node.thenBranch->accept(*this);
    if (node.elseBranch) node.elseBranch->accept(*this);
}

void SemanticAnalyzer::visit(WhileStmt& node) {
    node.condition->accept(*this);
    node.body->accept(*this);
}

void SemanticAnalyzer::visit(ForStmt& node) {
    symTable_.enterScope();
    if (node.init) node.init->accept(*this);
    if (node.condition) node.condition->accept(*this);
    if (node.update) node.update->accept(*this);
    if (node.body) node.body->accept(*this);
    symTable_.exitScope();
}

void SemanticAnalyzer::visit(ReturnStmt& node) {
    if (node.value) {
        node.value->accept(*this);
        if (currentFunctionReturnType_ == "void") {
            errors_.reportError("Cannot return a value from void function",
                               node.line, node.column, "Semantic");
        } else if (!isAssignmentCompatible(currentFunctionReturnType_, currentExprType_)) {
            errors_.reportError("Return type mismatch: expected '" +
                               currentFunctionReturnType_ + "', got '" +
                               currentExprType_ + "'", node.line, node.column, "Semantic");
        }
    }
}

void SemanticAnalyzer::visit(CoutStmt& node) {
    for (auto& expr : node.expressions) {
        if (expr) expr->accept(*this);
    }
}

void SemanticAnalyzer::visit(CinStmt& node) {
    for (auto& target : node.targets) {
        if (target) {
            target->accept(*this);
            // Verify target is an l-value (identifier or array access)
            if (!dynamic_cast<IdentifierExpr*>(target.get()) &&
                !dynamic_cast<ArrayAccessExpr*>(target.get())) {
                errors_.reportError("cin >> target must be a variable",
                                   node.line, node.column, "Semantic");
            }
        }
    }
}

// ── Expressions ───────────────────────────────────────────────────────────
void SemanticAnalyzer::visit(BinaryExpr& node) {
    node.left->accept(*this);
    std::string leftType = currentExprType_;
    node.right->accept(*this);
    std::string rightType = currentExprType_;
    currentExprType_ = resolveType(leftType, rightType, node.op, node.line, node.column);
}

void SemanticAnalyzer::visit(UnaryExpr& node) {
    node.operand->accept(*this);
    if (node.op == "!") {
        currentExprType_ = "bool";
    } else if (node.op == "-") {
        // Negation keeps the type
    } else if (node.op == "++" || node.op == "--") {
        if (currentExprType_ != "int" && currentExprType_ != "float") {
            errors_.reportError("Increment/decrement requires numeric type",
                               node.line, node.column, "Semantic");
        }
    }
}

void SemanticAnalyzer::visit(PostfixExpr& node) {
    node.operand->accept(*this);
    if (currentExprType_ != "int" && currentExprType_ != "float") {
        errors_.reportError("Postfix " + node.op + " requires numeric type",
                           node.line, node.column, "Semantic");
    }
}

void SemanticAnalyzer::visit(AssignExpr& node) {
    node.target->accept(*this);
    std::string targetType = currentExprType_;
    node.value->accept(*this);
    std::string valueType = currentExprType_;

    if (!isAssignmentCompatible(targetType, valueType)) {
        errors_.reportError("Cannot assign '" + valueType + "' to '" + targetType + "'",
                           node.line, node.column, "Semantic");
    }
    currentExprType_ = targetType;
}

void SemanticAnalyzer::visit(CallExpr& node) {
    auto sym = symTable_.lookup(node.name);
    if (!sym.has_value()) {
        errors_.reportError("Undeclared function '" + node.name + "'",
                           node.line, node.column, "Semantic");
        currentExprType_ = "int";
        return;
    }
    if (sym->kind != SymbolKind::FUNCTION) {
        errors_.reportError("'" + node.name + "' is not a function",
                           node.line, node.column, "Semantic");
    }

    // Check argument count
    if (node.args.size() != sym->paramTypes.size()) {
        errors_.reportError("Function '" + node.name + "' expects " +
                           std::to_string(sym->paramTypes.size()) + " args, got " +
                           std::to_string(node.args.size()),
                           node.line, node.column, "Semantic");
    }

    // Check argument types
    for (size_t i = 0; i < node.args.size() && i < sym->paramTypes.size(); i++) {
        node.args[i]->accept(*this);
        if (!isAssignmentCompatible(sym->paramTypes[i], currentExprType_)) {
            errors_.reportError("Argument " + std::to_string(i+1) + " type mismatch: expected '" +
                               sym->paramTypes[i] + "', got '" + currentExprType_ + "'",
                               node.line, node.column, "Semantic");
        }
    }

    currentExprType_ = sym->returnType.empty() ? sym->type : sym->returnType;
}

void SemanticAnalyzer::visit(ArrayAccessExpr& node) {
    auto sym = symTable_.lookup(node.name);
    if (!sym.has_value()) {
        errors_.reportError("Undeclared array '" + node.name + "'",
                           node.line, node.column, "Semantic");
        currentExprType_ = "int";
        return;
    }
    if (sym->kind != SymbolKind::ARRAY) {
        errors_.reportError("'" + node.name + "' is not an array",
                           node.line, node.column, "Semantic");
    }

    node.index->accept(*this);
    if (currentExprType_ != "int") {
        errors_.reportError("Array index must be integer, got '" + currentExprType_ + "'",
                           node.line, node.column, "Semantic");
    }
    currentExprType_ = sym->type;
}

void SemanticAnalyzer::visit(MemberAccessExpr& node) {
    node.object->accept(*this);
    std::string objType = currentExprType_;

    auto memberSym = symTable_.lookupClassMember(objType, node.member);
    if (!memberSym.has_value()) {
        errors_.reportError("Class '" + objType + "' has no member '" + node.member + "'",
                           node.line, node.column, "Semantic");
        currentExprType_ = "int";
        return;
    }

    // Check access level (private members only accessible within class)
    if (memberSym->accessLevel == "private" && currentClassName_ != objType) {
        errors_.reportError("Cannot access private member '" + node.member +
                           "' of class '" + objType + "'",
                           node.line, node.column, "Semantic");
    }
    currentExprType_ = memberSym->type;
}

void SemanticAnalyzer::visit(MethodCallExpr& node) {
    node.object->accept(*this);
    std::string objType = currentExprType_;

    auto memberSym = symTable_.lookupClassMember(objType, node.method);
    if (!memberSym.has_value()) {
        errors_.reportError("Class '" + objType + "' has no method '" + node.method + "'",
                           node.line, node.column, "Semantic");
        currentExprType_ = "int";
        return;
    }

    if (memberSym->accessLevel == "private" && currentClassName_ != objType) {
        errors_.reportError("Cannot access private method '" + node.method +
                           "' of class '" + objType + "'",
                           node.line, node.column, "Semantic");
    }

    for (auto& arg : node.args) {
        if (arg) arg->accept(*this);
    }
    currentExprType_ = memberSym->type;
}

void SemanticAnalyzer::visit(IdentifierExpr& node) {
    auto sym = symTable_.lookup(node.name);
    if (!sym.has_value()) {
        errors_.reportError("Undeclared variable '" + node.name + "'",
                           node.line, node.column, "Semantic");
        currentExprType_ = "int";
        return;
    }
    currentExprType_ = sym->type;
}

// ── Literals ──────────────────────────────────────────────────────────────
void SemanticAnalyzer::visit(IntLiteral&)    { currentExprType_ = "int"; }
void SemanticAnalyzer::visit(FloatLiteral&)  { currentExprType_ = "float"; }
void SemanticAnalyzer::visit(CharLiteral&)   { currentExprType_ = "char"; }
void SemanticAnalyzer::visit(StringLiteral&) { currentExprType_ = "string"; }
void SemanticAnalyzer::visit(BoolLiteral&)   { currentExprType_ = "bool"; }

} // namespace seedscan
