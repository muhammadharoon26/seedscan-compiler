/***************************************************************************
 * tac.hpp — Three-Address Code (TAC) Data Structures
 * -------------------------------------------------------------------------
 * COMPILER THEORY — Intermediate Representation:
 * Three-Address Code is a linear IR where each instruction has at most
 * three operands: result = arg1 OP arg2. It bridges the gap between the
 * high-level AST and low-level machine code.
 *
 * TAC is platform-independent, making it ideal for optimization passes
 * before targeting a specific architecture.
 *
 * Example: "x = a + b * c" becomes:
 *   t0 = b * c
 *   t1 = a + t0
 *   x  = t1
 ***************************************************************************/
#ifndef SEEDSCAN_TAC_HPP
#define SEEDSCAN_TAC_HPP

#include <string>
#include <vector>
#include <sstream>

namespace seedscan {

/*--------------------------------------------------------------------------
 * TACOp — All possible TAC operations.
 *------------------------------------------------------------------------*/
enum class TACOp {
    // Arithmetic
    ADD, SUB, MUL, DIV, MOD, NEG,
    // Comparison
    EQ, NEQ, LT, GT, LTE, GTE,
    // Logical
    AND, OR, NOT,
    // Assignment
    ASSIGN,
    // Control flow
    LABEL, GOTO, IF_GOTO, IF_FALSE_GOTO,
    // Functions
    PARAM, CALL, RETURN,
    // I/O
    WRITE, WRITELN, READ,
    // Arrays
    ARRAY_LOAD,   // result = base[index]
    ARRAY_STORE,  // base[index] = arg1
    // Member access
    MEMBER_LOAD,  // result = obj.member
    MEMBER_STORE, // obj.member = arg1
    // Increment/Decrement
    INC, DEC
};

/*--------------------------------------------------------------------------
 * TACInstruction — A single three-address code instruction.
 * Format: result = arg1 OP arg2  (some fields may be empty)
 *------------------------------------------------------------------------*/
struct TACInstruction {
    TACOp       op;
    std::string result;     // Destination (temp or variable)
    std::string arg1;       // First operand
    std::string arg2;       // Second operand (may be empty)

    TACInstruction(TACOp o, const std::string& r,
                   const std::string& a1 = "", const std::string& a2 = "")
        : op(o), result(r), arg1(a1), arg2(a2) {}

    // Convert to human-readable string
    std::string toString() const;
};

/*--------------------------------------------------------------------------
 * TACProgram — A complete TAC program (ordered list of instructions).
 *------------------------------------------------------------------------*/
class TACProgram {
public:
    // Add an instruction
    void emit(const TACInstruction& instr);
    void emit(TACOp op, const std::string& result,
              const std::string& arg1 = "", const std::string& arg2 = "");

    // Generate fresh temporary variable: t0, t1, t2, ...
    std::string newTemp();

    // Generate fresh label: L0, L1, L2, ...
    std::string newLabel();

    // Get all instructions
    const std::vector<TACInstruction>& instructions() const { return instructions_; }

    // Convert entire program to string
    std::string toString() const;

private:
    std::vector<TACInstruction> instructions_;
    int tempCounter_  = 0;
    int labelCounter_ = 0;
};

} // namespace seedscan

#endif // SEEDSCAN_TAC_HPP
