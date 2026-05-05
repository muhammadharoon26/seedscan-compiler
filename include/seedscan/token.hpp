/***************************************************************************
 * token.hpp — Token Type Definitions for the SeedScan Mini-C++ Compiler
 * -------------------------------------------------------------------------
 * COMPILER THEORY:
 * A token is the smallest meaningful unit of a programming language. The
 * lexer (scanner) reads raw characters and groups them into tokens. Each
 * token has a TYPE (what category it belongs to) and a LEXEME (the actual
 * text from the source code).
 *
 * Example: "int x = 42;" produces tokens:
 *   { INT_TYPE, "int" } { IDENTIFIER, "x" } { ASSIGN, "=" }
 *   { INT_LITERAL, "42" } { SEMICOLON, ";" }
 *
 * This file defines all possible token types for Mini-C++ and the Token
 * struct that carries type + lexeme + source location (line/column) for
 * error reporting.
 ***************************************************************************/
#ifndef SEEDSCAN_TOKEN_HPP
#define SEEDSCAN_TOKEN_HPP

#include <string>
#include <unordered_map>

namespace seedscan {

/*--------------------------------------------------------------------------
 * TokenType — Enumeration of every distinct token in Mini-C++.
 *
 * Grouped by category for clarity:
 *   1. Keywords (language-reserved words)
 *   2. Literal values (numbers, characters, strings, booleans)
 *   3. Identifiers (user-defined names)
 *   4. Operators (arithmetic, comparison, logical, assignment, unary)
 *   5. Delimiters (punctuation, brackets)
 *   6. I/O stream operators
 *   7. Special tokens (EOF, error)
 *------------------------------------------------------------------------*/
enum class TokenType {
    // ── Keywords ──────────────────────────────────────────────────────
    KW_INT,         // "int"
    KW_FLOAT,       // "float"
    KW_CHAR,        // "char"
    KW_BOOL,        // "bool"
    KW_VOID,        // "void"
    KW_IF,          // "if"
    KW_ELSE,        // "else"
    KW_WHILE,       // "while"
    KW_FOR,         // "for"
    KW_RETURN,      // "return"
    KW_CLASS,       // "class"
    KW_PUBLIC,      // "public"
    KW_PRIVATE,     // "private"
    KW_TRUE,        // "true"
    KW_FALSE,       // "false"
    KW_COUT,        // "cout"
    KW_CIN,         // "cin"
    KW_ENDL,        // "endl"
    KW_NEW,         // "new"

    // ── Literals ──────────────────────────────────────────────────────
    LIT_INT,        // e.g., 42, 0, 1000
    LIT_FLOAT,      // e.g., 3.14, 0.5
    LIT_CHAR,       // e.g., 'a', '\n'
    LIT_STRING,     // e.g., "hello world"

    // ── Identifiers ───────────────────────────────────────────────────
    IDENTIFIER,     // user-defined names: myVar, Counter, _temp

    // ── Arithmetic Operators ──────────────────────────────────────────
    OP_PLUS,        // +
    OP_MINUS,       // -
    OP_STAR,        // *
    OP_SLASH,       // /
    OP_MOD,         // %

    // ── Comparison Operators ──────────────────────────────────────────
    OP_EQ,          // ==
    OP_NEQ,         // !=
    OP_LT,          // <
    OP_GT,          // >
    OP_LTE,         // <=
    OP_GTE,         // >=

    // ── Logical Operators ─────────────────────────────────────────────
    OP_AND,         // &&
    OP_OR,          // ||
    OP_NOT,         // !

    // ── Assignment Operators ──────────────────────────────────────────
    OP_ASSIGN,      // =
    OP_PLUS_ASSIGN, // +=
    OP_MINUS_ASSIGN,// -=
    OP_STAR_ASSIGN, // *=
    OP_SLASH_ASSIGN,// /=

    // ── Increment / Decrement ─────────────────────────────────────────
    OP_INCREMENT,   // ++
    OP_DECREMENT,   // --

    // ── I/O Stream Operators ──────────────────────────────────────────
    OP_LSHIFT,      // <<  (used for cout <<)
    OP_RSHIFT,      // >>  (used for cin >>)

    // ── Delimiters ────────────────────────────────────────────────────
    LPAREN,         // (
    RPAREN,         // )
    LBRACE,         // {
    RBRACE,         // }
    LBRACKET,       // [
    RBRACKET,       // ]
    SEMICOLON,      // ;
    COMMA,          // ,
    DOT,            // .
    COLON,          // :

    // ── Special ───────────────────────────────────────────────────────
    TOK_EOF,        // End of file
    TOK_ERROR       // Unrecognized character
};

/*--------------------------------------------------------------------------
 * Token — Carries all information about a single lexical unit.
 *
 * The line and column fields enable precise error messages like:
 *   "Error at line 10, column 5: Undefined variable 'x'"
 *------------------------------------------------------------------------*/
struct Token {
    TokenType   type;       // Category of this token
    std::string lexeme;     // The actual text from source code
    int         line;       // 1-indexed line number
    int         column;     // 1-indexed column number

    Token() : type(TokenType::TOK_EOF), lexeme(""), line(0), column(0) {}
    Token(TokenType t, const std::string& l, int ln, int col)
        : type(t), lexeme(l), line(ln), column(col) {}
};

/*--------------------------------------------------------------------------
 * tokenTypeToString — Converts a TokenType enum to a human-readable string.
 * Used for debugging (--tokens flag) and error messages.
 *------------------------------------------------------------------------*/
inline std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::KW_INT:         return "KW_INT";
        case TokenType::KW_FLOAT:       return "KW_FLOAT";
        case TokenType::KW_CHAR:        return "KW_CHAR";
        case TokenType::KW_BOOL:        return "KW_BOOL";
        case TokenType::KW_VOID:        return "KW_VOID";
        case TokenType::KW_IF:          return "KW_IF";
        case TokenType::KW_ELSE:        return "KW_ELSE";
        case TokenType::KW_WHILE:       return "KW_WHILE";
        case TokenType::KW_FOR:         return "KW_FOR";
        case TokenType::KW_RETURN:      return "KW_RETURN";
        case TokenType::KW_CLASS:       return "KW_CLASS";
        case TokenType::KW_PUBLIC:      return "KW_PUBLIC";
        case TokenType::KW_PRIVATE:     return "KW_PRIVATE";
        case TokenType::KW_TRUE:        return "KW_TRUE";
        case TokenType::KW_FALSE:       return "KW_FALSE";
        case TokenType::KW_COUT:        return "KW_COUT";
        case TokenType::KW_CIN:         return "KW_CIN";
        case TokenType::KW_ENDL:        return "KW_ENDL";
        case TokenType::KW_NEW:         return "KW_NEW";
        case TokenType::LIT_INT:        return "LIT_INT";
        case TokenType::LIT_FLOAT:      return "LIT_FLOAT";
        case TokenType::LIT_CHAR:       return "LIT_CHAR";
        case TokenType::LIT_STRING:     return "LIT_STRING";
        case TokenType::IDENTIFIER:     return "IDENTIFIER";
        case TokenType::OP_PLUS:        return "OP_PLUS";
        case TokenType::OP_MINUS:       return "OP_MINUS";
        case TokenType::OP_STAR:        return "OP_STAR";
        case TokenType::OP_SLASH:       return "OP_SLASH";
        case TokenType::OP_MOD:         return "OP_MOD";
        case TokenType::OP_EQ:          return "OP_EQ";
        case TokenType::OP_NEQ:         return "OP_NEQ";
        case TokenType::OP_LT:          return "OP_LT";
        case TokenType::OP_GT:          return "OP_GT";
        case TokenType::OP_LTE:         return "OP_LTE";
        case TokenType::OP_GTE:         return "OP_GTE";
        case TokenType::OP_AND:         return "OP_AND";
        case TokenType::OP_OR:          return "OP_OR";
        case TokenType::OP_NOT:         return "OP_NOT";
        case TokenType::OP_ASSIGN:      return "OP_ASSIGN";
        case TokenType::OP_PLUS_ASSIGN: return "OP_PLUS_ASSIGN";
        case TokenType::OP_MINUS_ASSIGN:return "OP_MINUS_ASSIGN";
        case TokenType::OP_STAR_ASSIGN: return "OP_STAR_ASSIGN";
        case TokenType::OP_SLASH_ASSIGN:return "OP_SLASH_ASSIGN";
        case TokenType::OP_INCREMENT:   return "OP_INCREMENT";
        case TokenType::OP_DECREMENT:   return "OP_DECREMENT";
        case TokenType::OP_LSHIFT:      return "OP_LSHIFT";
        case TokenType::OP_RSHIFT:      return "OP_RSHIFT";
        case TokenType::LPAREN:         return "LPAREN";
        case TokenType::RPAREN:         return "RPAREN";
        case TokenType::LBRACE:         return "LBRACE";
        case TokenType::RBRACE:         return "RBRACE";
        case TokenType::LBRACKET:       return "LBRACKET";
        case TokenType::RBRACKET:       return "RBRACKET";
        case TokenType::SEMICOLON:      return "SEMICOLON";
        case TokenType::COMMA:          return "COMMA";
        case TokenType::DOT:            return "DOT";
        case TokenType::COLON:          return "COLON";
        case TokenType::TOK_EOF:        return "TOK_EOF";
        case TokenType::TOK_ERROR:      return "TOK_ERROR";
        default:                        return "UNKNOWN";
    }
}

} // namespace seedscan

#endif // SEEDSCAN_TOKEN_HPP
