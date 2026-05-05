# SeedScan Compiler — TDD Specification (Deliverable 2)

## Test-Driven Development Strategy

Each compiler phase has its own test suite using Google Test. Tests are organized by functionality and cover both positive (valid input) and negative (error detection) cases.

---

## 1. Lexer Test Suite (`tests/lexer_test.cpp`)

### 1.1 Keyword Recognition
| Test Case | Input | Expected Token |
|---|---|---|
| Integer type | `int` | `INT_TYPE` |
| Float type | `float` | `FLOAT_TYPE` |
| Char type | `char` | `CHAR_TYPE` |
| Bool type | `bool` | `BOOL_TYPE` |
| Void type | `void` | `VOID_TYPE` |
| Control flow | `if else while for return` | Respective keyword tokens |
| Class keywords | `class public private` | Respective keyword tokens |
| I/O keywords | `cout cin endl` | Respective keyword tokens |
| Bool literals | `true false` | `TRUE_LIT` `FALSE_LIT` |

### 1.2 Operator Recognition
| Test Case | Input | Expected Tokens |
|---|---|---|
| Arithmetic | `+ - * / %` | `PLUS MINUS STAR SLASH MOD` |
| Comparison | `== != < > <= >=` | `EQ NEQ LT GT LTE GTE` |
| Logical | `&& \|\| !` | `AND OR NOT` |
| Assignment | `= += -= *= /=` | `ASSIGN PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN SLASH_ASSIGN` |
| Inc/Dec | `++ --` | `INCREMENT DECREMENT` |
| Stream | `<< >>` | `LSHIFT RSHIFT` |

### 1.3 Literal Recognition
| Test Case | Input | Expected |
|---|---|---|
| Integer | `42` | `INT_LITERAL`, value=42 |
| Float | `3.14` | `FLOAT_LITERAL`, value≈3.14 |
| Character | `'a'` | `CHAR_LITERAL`, value='a' |
| String | `"hello"` | `STRING_LITERAL`, value="hello" |

### 1.4 Comment Handling
| Test Case | Input | Expected |
|---|---|---|
| Single-line | `int // comment\nfloat` | `INT_TYPE FLOAT_TYPE` |
| Multi-line | `int /* ... */ float` | `INT_TYPE FLOAT_TYPE` |

### 1.5 Error Handling
| Test Case | Input | Expected |
|---|---|---|
| Invalid char | `@` | `ERROR_TOKEN` |

---

## 2. Parser Test Suite (`tests/parser_test.cpp`)

### 2.1 Declaration Parsing
| Test Case | Input | Expected AST Node |
|---|---|---|
| Variable decl | `int x = 42;` | `VarDeclaration(int, x, IntLiteral(42))` |
| Array decl | `int arr[10];` | `ArrayDeclaration(int, arr, 10)` |
| Function decl | `int add(int a, int b) {...}` | `FunDeclaration` with 2 params |
| Class decl | `class Foo { public: int x; };` | `ClassDeclaration` with member |

### 2.2 Statement Parsing
| Test Case | Input | Expected AST Node |
|---|---|---|
| If/else | `if (x > 5) {...} else {...}` | `IfStmt` with both branches |
| While loop | `while (x < 10) {...}` | `WhileStmt` |
| For loop | `for (int i=0; i<10; i++) {...}` | `ForStmt` |
| Return | `return a + b;` | `ReturnStmt` with expression |

### 2.3 Expression Precedence
| Test Case | Input | Expected Parse |
|---|---|---|
| Mult before add | `2 + 3 * 4` | `BinaryExpr(+, 2, BinaryExpr(*, 3, 4))` |

### 2.4 I/O Parsing
| Test Case | Input | Expected AST Node |
|---|---|---|
| Cout chain | `cout << "hi" << endl;` | `CoutStmt` with 2 expressions |
| Cin chain | `cin >> x;` | `CinStmt` with 1 target |

---

## 3. Semantic Analyzer Test Suite (`tests/semantic_test.cpp`)

### 3.1 Scope Resolution
| Test Case | Input | Expected |
|---|---|---|
| Undeclared var | `x = 10;` (no decl) | Error: undeclared variable |
| Declared var | `int x = 10;` | No error |
| Redeclaration | `int x; int x;` (same scope) | Error: redeclaration |
| Shadowing | `int x; { int x; }` | No error (nested scope) |

### 3.2 Type Checking
| Test Case | Input | Expected |
|---|---|---|
| Int arithmetic | `int c = a + b;` | No error |
| Int-float promotion | `float c = intVar + floatVar;` | No error (promotes) |
| Type mismatch | `int x = "hello";` | Error: type mismatch |

### 3.3 Function Validation
| Test Case | Input | Expected |
|---|---|---|
| Undeclared func | `foo();` | Error: undeclared function |
| Wrong arg count | `add(1)` (expects 2) | Error: arg count mismatch |
| Correct call | `add(1, 2)` | No error |

### 3.4 Return Type
| Test Case | Input | Expected |
|---|---|---|
| Void returns value | `void f() { return 42; }` | Error |
| Correct return | `int f() { return 42; }` | No error |

### 3.5 Array Validation
| Test Case | Input | Expected |
|---|---|---|
| Valid access | `arr[0]` | No error |
| Non-int index | `arr[3.14]` | Error: index must be integer |

---

## 4. IR Generator Test Suite (`tests/ir_test.cpp`)

### 4.1 Assignment & Arithmetic
| Test Case | Input | Expected TAC |
|---|---|---|
| Simple assign | `int x = 42;` | `x = 42` |
| Addition | `int c = a + b;` | `t0 = a + b` `c = t0` |

### 4.2 Control Flow
| Test Case | Input | Expected TAC Pattern |
|---|---|---|
| If/else | `if (cond) {...} else {...}` | `if_false ... goto L` + labels |
| While loop | `while (cond) {...}` | Label + `if_false` + `goto` back |
| For loop | `for (init; cond; upd) {...}` | Init + label + condition + body + update + jump |

### 4.3 Functions
| Test Case | Input | Expected TAC |
|---|---|---|
| Function call | `add(1, 2)` | `param 1` `param 2` `call add, 2` |
| Return | `return x;` | `return x` |

### 4.4 I/O
| Test Case | Input | Expected TAC |
|---|---|---|
| Cout | `cout << x << endl;` | `write x` `writeln` |
| Cin | `cin >> x;` | `read x` |

---

## Running Tests

```powershell
cd d:\seedscan-compiler\build
cmake --build .
ctest --output-on-failure
```
