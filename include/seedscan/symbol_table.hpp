/***************************************************************************
 * symbol_table.hpp — Scoped Symbol Table for Semantic Analysis
 * -------------------------------------------------------------------------
 * COMPILER THEORY — Symbol Table:
 * The symbol table is a central data structure used across compiler phases.
 * It maps identifiers (variable/function/class names) to their attributes
 * (type, scope level, kind). It supports NESTED SCOPES via a scope stack:
 *
 * Scope Stack Example:
 *   [Global] → [Function foo] → [If-block]
 *
 * Lookup searches from innermost scope outward (lexical scoping).
 * Declaration adds to the CURRENT (innermost) scope only.
 ***************************************************************************/
#ifndef SEEDSCAN_SYMBOL_TABLE_HPP
#define SEEDSCAN_SYMBOL_TABLE_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

namespace seedscan {

/*--------------------------------------------------------------------------
 * SymbolKind — What kind of entity this symbol represents.
 *------------------------------------------------------------------------*/
enum class SymbolKind {
    VARIABLE,
    FUNCTION,
    PARAMETER,
    ARRAY,
    CLASS,
    MEMBER_VAR,
    MEMBER_FUNC
};

/*--------------------------------------------------------------------------
 * Symbol — A single entry in the symbol table.
 *------------------------------------------------------------------------*/
struct Symbol {
    std::string name;
    std::string type;       // "int", "float", "char", "bool", "void", or class name
    SymbolKind  kind;
    int         scopeLevel;
    int         line;       // Declaration line (for error messages)
    int         column;

    // Function-specific: parameter types for call validation
    std::vector<std::string> paramTypes;
    std::string returnType;

    // Array-specific
    int arraySize = 0;

    // Class-specific: access level
    std::string accessLevel; // "public" or "private"

    Symbol() : kind(SymbolKind::VARIABLE), scopeLevel(0), line(0), column(0) {}
    Symbol(const std::string& n, const std::string& t, SymbolKind k, int scope,
           int ln = 0, int col = 0)
        : name(n), type(t), kind(k), scopeLevel(scope), line(ln), column(col) {}
};

/*--------------------------------------------------------------------------
 * SymbolTable — Manages nested scopes and symbol lookup.
 *------------------------------------------------------------------------*/
class SymbolTable {
public:
    SymbolTable();

    // Scope management
    void enterScope();
    void exitScope();
    int  currentScopeLevel() const;

    // Symbol operations
    bool declare(const Symbol& symbol);
    std::optional<Symbol> lookup(const std::string& name) const;
    std::optional<Symbol> lookupCurrentScope(const std::string& name) const;

    // Class member lookup: look up a member within a class
    std::optional<Symbol> lookupClassMember(const std::string& className,
                                            const std::string& memberName) const;

    // Register class members for member-access validation
    void declareClassMember(const std::string& className, const Symbol& member);

private:
    // Each scope is a map from name to Symbol
    using Scope = std::unordered_map<std::string, Symbol>;

    std::vector<Scope> scopes_;          // Scope stack
    int scopeLevel_;

    // Class member tables: className -> { memberName -> Symbol }
    std::unordered_map<std::string, std::unordered_map<std::string, Symbol>> classMembers_;
};

} // namespace seedscan

#endif // SEEDSCAN_SYMBOL_TABLE_HPP
