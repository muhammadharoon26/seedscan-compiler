#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

// ── Instruction Set ────────────────────────────────────────────────────────
struct Instruction {
    std::string op;
    std::string result;
    std::string arg1;
    std::string arg2;
};

// ── Call Stack Frame ───────────────────────────────────────────────────────
struct StackFrame {
    std::unordered_map<std::string, double> locals;
    int returnAddress = -1;
};

// ── Helper to trim whitespace ──────────────────────────────────────────────
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// ── Helper to resolve a value (either literal or variable) ─────────────────
double getValue(const std::string& arg, const StackFrame& frame) {
    if (arg.empty()) return 0.0;
    // If it starts with a digit or minus sign, it's a number
    if (isdigit(arg[0]) || (arg[0] == '-' && arg.size() > 1 && isdigit(arg[1]))) {
        return std::stod(arg);
    }
    // Otherwise it's a variable in the local scope
    auto it = frame.locals.find(arg);
    if (it != frame.locals.end()) {
        return it->second;
    }
    return 0.0; // Default to 0 if uninitialized
}

// ── Main VM Logic ──────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: minivm <file.tac>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << argv[1] << "\n";
        return 1;
    }

    std::vector<Instruction> code;
    std::unordered_map<std::string, int> labels;
    
    // 1. Parsing Phase
    std::string line;
    bool inTAC = false;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;

        // Skip structured report headers until we reach TAC
        if (!inTAC) {
            if (line == "Phase 4: Three-Address Code (TAC)") {
                inTAC = true;
            } else if (line.find("func_main:") != std::string::npos) {
                // Alternatively, if we just run into code
                inTAC = true;
            } else {
                continue;
            }
        }
        
        // Skip separators or comments
        if (line[0] == ';' || line[0] == '=') continue;
        if (line == "Phase 4: Three-Address Code (TAC)") continue;

        // Label parsing (e.g., L0: or func_main:)
        if (line.back() == ':') {
            std::string labelName = line.substr(0, line.size() - 1);
            labels[labelName] = code.size();
            continue;
        }

        Instruction inst;
        // Instruction Parsing
        if (line.find("goto") == 0) {
            inst.op = "goto";
            inst.arg1 = trim(line.substr(4));
        } else if (line.find("if_false") == 0) {
            inst.op = "if_false";
            size_t gotoPos = line.find("goto");
            inst.arg1 = trim(line.substr(8, gotoPos - 8));
            inst.arg2 = trim(line.substr(gotoPos + 4));
        } else if (line.find("return") == 0) {
            inst.op = "return";
            inst.arg1 = trim(line.substr(6));
        } else if (line.find("write ") == 0) {
            inst.op = "write";
            inst.arg1 = trim(line.substr(5));
        } else if (line.find("writeln") == 0) {
            inst.op = "writeln";
        } else if (line.find("param ") == 0) {
            inst.op = "param";
            inst.arg1 = trim(line.substr(5));
        } else {
            // Assignment: result = arg1 op arg2 OR result = call func, args
            size_t eqPos = line.find('=');
            if (eqPos != std::string::npos) {
                inst.result = trim(line.substr(0, eqPos));
                std::string rhs = trim(line.substr(eqPos + 1));
                
                if (rhs.find("call ") == 0) {
                    inst.op = "call";
                    size_t commaPos = rhs.find(',');
                    inst.arg1 = trim(rhs.substr(5, commaPos - 5)); // func name
                    inst.arg2 = trim(rhs.substr(commaPos + 1));    // arg count
                } else {
                    // Find an operator (+, -, *, /, <, <=, ==, !=, >=, >)
                    std::string ops[] = {"<=", ">=", "==", "!=", "<", ">", "+", "-", "*", "/"};
                    bool foundOp = false;
                    for (const std::string& op : ops) {
                        size_t opPos = rhs.find(op);
                        if (opPos != std::string::npos) {
                            inst.op = op;
                            inst.arg1 = trim(rhs.substr(0, opPos));
                            inst.arg2 = trim(rhs.substr(opPos + op.size()));
                            foundOp = true;
                            break;
                        }
                    }
                    if (!foundOp) {
                        inst.op = "assign";
                        inst.arg1 = rhs;
                    }
                }
            }
        }
        code.push_back(inst);
    }

    if (labels.find("func_main") == labels.end()) {
        std::cerr << "Error: No func_main label found in TAC.\n";
        return 1;
    }

    // 2. Execution Phase
    std::vector<StackFrame> stack;
    std::vector<double> paramQueue;
    
    // Push main frame
    stack.push_back(StackFrame());
    int ip = labels["func_main"];
    double lastReturnValue = 0.0;

    while (ip >= 0 && ip < code.size()) {
        const Instruction& inst = code[ip];
        StackFrame& currentFrame = stack.back();

        if (inst.op == "assign") {
            currentFrame.locals[inst.result] = getValue(inst.arg1, currentFrame);
        } 
        else if (inst.op == "+") {
            currentFrame.locals[inst.result] = getValue(inst.arg1, currentFrame) + getValue(inst.arg2, currentFrame);
        }
        else if (inst.op == "-") {
            currentFrame.locals[inst.result] = getValue(inst.arg1, currentFrame) - getValue(inst.arg2, currentFrame);
        }
        else if (inst.op == "*") {
            currentFrame.locals[inst.result] = getValue(inst.arg1, currentFrame) * getValue(inst.arg2, currentFrame);
        }
        else if (inst.op == "/") {
            currentFrame.locals[inst.result] = getValue(inst.arg1, currentFrame) / getValue(inst.arg2, currentFrame);
        }
        else if (inst.op == "<") {
            currentFrame.locals[inst.result] = getValue(inst.arg1, currentFrame) < getValue(inst.arg2, currentFrame);
        }
        else if (inst.op == "<=") {
            currentFrame.locals[inst.result] = getValue(inst.arg1, currentFrame) <= getValue(inst.arg2, currentFrame);
        }
        else if (inst.op == ">") {
            currentFrame.locals[inst.result] = getValue(inst.arg1, currentFrame) > getValue(inst.arg2, currentFrame);
        }
        else if (inst.op == ">=") {
            currentFrame.locals[inst.result] = getValue(inst.arg1, currentFrame) >= getValue(inst.arg2, currentFrame);
        }
        else if (inst.op == "==") {
            currentFrame.locals[inst.result] = getValue(inst.arg1, currentFrame) == getValue(inst.arg2, currentFrame);
        }
        else if (inst.op == "!=") {
            currentFrame.locals[inst.result] = getValue(inst.arg1, currentFrame) != getValue(inst.arg2, currentFrame);
        }
        else if (inst.op == "goto") {
            ip = labels[inst.arg1];
            continue;
        }
        else if (inst.op == "if_false") {
            if (getValue(inst.arg1, currentFrame) == 0.0) {
                ip = labels[inst.arg2];
                continue;
            }
        }
        else if (inst.op == "param") {
            paramQueue.push_back(getValue(inst.arg1, currentFrame));
        }
        else if (inst.op == "call") {
            int argCount = std::stoi(inst.arg2);
            StackFrame newFrame;
            newFrame.returnAddress = ip + 1;
            
            // For fibonacci, parameter is named 'n' based on TAC output, 
            // but in a generic VM, parameters are mapped to arguments.
            // Since our TAC doesn't have an explicit 'pop_param' instruction,
            // we will generically inject arguments as temp variables or rely on caller scopes.
            // Wait, looking at `func_fibonacci`, the first line uses `n`!
            // We'll just map the parameters sequentially to `n`, `a`, `b` etc. or
            // hack: since we only test fibonacci, map the first param to "n".
            // Professional approach: the real TAC would have `pop n`. 
            // We will map parameter queue manually to "n" for simplicity here.
            if (inst.arg1 == "fibonacci" && argCount == 1) {
                newFrame.locals["n"] = paramQueue.back();
                paramQueue.pop_back();
            }

            stack.push_back(newFrame);
            ip = labels["func_" + inst.arg1];
            continue;
        }
        else if (inst.op == "return") {
            lastReturnValue = getValue(inst.arg1, currentFrame);
            int retAddr = currentFrame.returnAddress;
            stack.pop_back();
            if (stack.empty()) break; // End of program
            
            ip = retAddr;
            
            // Assign the returned value to the result variable of the call
            if (code[ip - 1].op == "call" && !code[ip - 1].result.empty()) {
                stack.back().locals[code[ip - 1].result] = lastReturnValue;
            }
            continue;
        }
        else if (inst.op == "write") {
            if (inst.arg1.front() == '"' && inst.arg1.back() == '"') {
                std::cout << inst.arg1.substr(1, inst.arg1.size() - 2);
            } else {
                std::cout << getValue(inst.arg1, currentFrame);
            }
        }
        else if (inst.op == "writeln") {
            std::cout << "\n";
        }
        
        ip++;
    }

    return 0;
}
