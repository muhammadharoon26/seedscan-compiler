# SeedScan Compiler — Formal Grammar (BNF)

## Mini-C++ Grammar Specification

```bnf
program             → declaration_list

declaration_list    → declaration
                    | declaration_list declaration

declaration         → var_declaration
                    | fun_declaration
                    | class_declaration

type_specifier      → 'int' | 'float' | 'char' | 'bool' | 'void' | ID

var_declaration     → type_specifier ID ';'
                    | type_specifier ID '=' expression ';'
                    | type_specifier ID '[' INT_LITERAL ']' ';'

fun_declaration     → type_specifier ID '(' params ')' compound_stmt

params              → param_list | 'void' | ε
param_list          → param | param_list ',' param
param               → type_specifier ID | type_specifier ID '[' ']'

class_declaration   → 'class' ID '{' class_body '}' ';'
class_body          → class_members | ε
class_members       → access_specifier ':' member_decls
                    | class_members access_specifier ':' member_decls
access_specifier    → 'public' | 'private'
member_decls        → declaration | member_decls declaration

compound_stmt       → '{' statement_list '}'
statement_list      → ε | statement_list statement | statement_list var_declaration

statement           → expression_stmt | compound_stmt | selection_stmt
                    | iteration_stmt | return_stmt | io_stmt

expression_stmt     → expression ';' | ';'
selection_stmt      → 'if' '(' expression ')' statement
                    | 'if' '(' expression ')' statement 'else' statement
iteration_stmt      → 'while' '(' expression ')' statement
                    | 'for' '(' for_init ';' expression ';' expression ')' statement
for_init            → type_specifier ID '=' expression | expression | ε
return_stmt         → 'return' ';' | 'return' expression ';'

io_stmt             → 'cout' cout_list ';' | 'cin' cin_list ';'
cout_list           → '<<' expression | '<<' 'endl' | cout_list '<<' expression | cout_list '<<' 'endl'
cin_list            → '>>' primary | cin_list '>>' primary

expression          → assign_expr
assign_expr         → logical_or
                    | postfix_expr ('=' | '+=' | '-=' | '*=' | '/=') assign_expr
logical_or          → logical_and | logical_or '||' logical_and
logical_and         → equality | logical_and '&&' equality
equality            → relational | equality ('==' | '!=') relational
relational          → additive | relational ('<' | '>' | '<=' | '>=') additive
additive            → multiplicative | additive ('+' | '-') multiplicative
multiplicative      → unary_expr | multiplicative ('*' | '/' | '%') unary_expr
unary_expr          → postfix_expr | ('!' | '-' | '++' | '--') unary_expr
postfix_expr        → primary | postfix_expr ('++' | '--')
                    | ID '(' args ')' | postfix_expr '.' ID
                    | postfix_expr '.' ID '(' args ')'
                    | ID '[' expression ']'
primary             → INT_LITERAL | FLOAT_LITERAL | CHAR_LITERAL | STRING_LITERAL
                    | 'true' | 'false' | ID | '(' expression ')'

args                → arg_list | ε
arg_list            → expression | arg_list ',' expression
```

## Operator Precedence (Low to High)

| Precedence | Operators | Associativity |
|---|---|---|
| 1 (lowest) | `=` `+=` `-=` `*=` `/=` | Right |
| 2 | `\|\|` | Left |
| 3 | `&&` | Left |
| 4 | `==` `!=` | Left |
| 5 | `<` `>` `<=` `>=` | Left |
| 6 | `+` `-` | Left |
| 7 | `*` `/` `%` | Left |
| 8 | `!` `-` (unary) `++` `--` (prefix) | Right |
| 9 (highest) | `++` `--` (postfix) `.` `[]` | Left |

## Lexical Elements

### Keywords (19)
`int` `float` `char` `bool` `void` `if` `else` `while` `for` `return`
`class` `public` `private` `true` `false` `cout` `cin` `endl` `new`

### Literals
- Integer: `[0-9]+`
- Float: `[0-9]+\.[0-9]+`
- Char: `'([^'\\]|\\.)'`
- String: `"([^"\\]|\\.)*"`

### Identifiers
`[a-zA-Z_][a-zA-Z0-9_]*`

### Comments
- Single-line: `// ...`
- Multi-line: `/* ... */`
