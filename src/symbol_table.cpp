/***************************************************************************
 * symbol_table.cpp — Scoped Symbol Table Implementation
 ***************************************************************************/
#include "seedscan/symbol_table.hpp"

namespace seedscan {

SymbolTable::SymbolTable() : scopeLevel_(0) {
    // Start with the global scope
    scopes_.emplace_back();
}

void SymbolTable::enterScope() {
    scopeLevel_++;
    scopes_.emplace_back();
}

void SymbolTable::exitScope() {
    if (scopes_.size() > 1) {
        scopes_.pop_back();
        scopeLevel_--;
    }
}

int SymbolTable::currentScopeLevel() const {
    return scopeLevel_;
}

bool SymbolTable::declare(const Symbol& symbol) {
    // Check for redeclaration in the CURRENT scope only
    auto& currentScope = scopes_.back();
    if (currentScope.find(symbol.name) != currentScope.end()) {
        return false; // Already declared in this scope
    }
    Symbol sym = symbol;
    sym.scopeLevel = scopeLevel_;
    currentScope[symbol.name] = sym;
    return true;
}

std::optional<Symbol> SymbolTable::lookup(const std::string& name) const {
    // Search from innermost scope outward (lexical scoping)
    for (int i = static_cast<int>(scopes_.size()) - 1; i >= 0; i--) {
        auto it = scopes_[i].find(name);
        if (it != scopes_[i].end()) {
            return it->second;
        }
    }
    return std::nullopt;
}

std::optional<Symbol> SymbolTable::lookupCurrentScope(const std::string& name) const {
    auto it = scopes_.back().find(name);
    if (it != scopes_.back().end()) {
        return it->second;
    }
    return std::nullopt;
}

void SymbolTable::declareClassMember(const std::string& className, const Symbol& member) {
    classMembers_[className][member.name] = member;
}

std::optional<Symbol> SymbolTable::lookupClassMember(const std::string& className,
                                                      const std::string& memberName) const {
    auto classIt = classMembers_.find(className);
    if (classIt == classMembers_.end()) return std::nullopt;

    auto memberIt = classIt->second.find(memberName);
    if (memberIt == classIt->second.end()) return std::nullopt;

    return memberIt->second;
}

} // namespace seedscan
