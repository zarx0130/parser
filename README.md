# C Lexer & Parser 
Parser and interpreter for C-like language, built in C implementing lexical analysis, syntax parsing, and basic interpretation using symbol tables and function management (*build on miniLexer)

# Features
Lexical analysis: tokenizes input source code (int/float/operator/reserved/etc)
Parser: parses expressions with operator precedence (loosely built on recursive descent)
Symbol Tables: manages global and local variables with scoping
Function Support: function declarations, parameters, and calls
Control Structures: if/else, while, for loops
Expression Evaluation: arithmetic, logic, comparison, and assignment operators
Type Support: int, float, char with type checking
Variable Scoping: local and global scope management
Error Handling: syntax errors

# Language Grammar Rules
program → statement*
statement → declaration | assignment | if | while | for | return | block | ";"
declaration → type identifier ("=" expression)? ";"
           | type identifier "(" parameters? ")" block
assignment → identifier ("=" | "+=" | "-=" | "*=" | "/=" | "%=") expression ";"
expression → logicalOr
logicalOr → logicalAnd ("||" logicalAnd)*
logicalAnd → equality ("&&" equality)*
equality → comparison (("==" | "!=") comparison)*
comparison → additive (("<" | ">" | "<=" | ">=") additive)*
additive → multiplicative (("+" | "-") multiplicative)*
multiplicative → unary (("*" | "/" | "%") unary)*
unary → ("!" | "-" | "++" | "--")? primary
primary → number | identifier | "(" expression ")" | functionCall
block → "{" statement* "}"
if → "if" "(" expression ")" statement ("else" statement)?
while → "while" "(" expression ")" statement
for → "for" "(" (declaration | expression? ";") expression? ";" expression? ")" statement
return → "return" expression? ";"

# Structure
parser.c - main parser/interpreter implementation (symbol/function table management, expression evaluation, and control flow parsing)

lexer.c - lexical analyzer (tokenization, character stream  management, and reserved word/operator recognition)

lexer.h - header file w/ token definitions

.txt files - demo files demonstrating program features

# Compilation &  Usage
gcc -o parser parser.c lexer.c

./parser <source_file.txt>

# Example Output
cc -o parser  parser.c lexer.c
./parser demoDeclaration.txt

Parsing successful

Global Symbol Table:

Type    ID      Value
----    --      ----
char    xray

Function Table:
Return  Name    Params
------  ----    ------


 ./parser demo.txt
Parsing successful

Global Symbol Table:

Type    ID      Value
----    --      ----
int     xray    40

Function Table:
Return  Name    Params
------  ----    ------

 ./parser demoLocal.txt
Parsing successful

Global Symbol Table:

Type    ID      Value
----    --      ----

Function Table:
Return  Name    Params
------  ----    ------
int     main

 ./parser demoEntry.txt
Parsing successful

Global Symbol Table:

Type    ID      Value
----    --      ----
int     pear    2
int     apple   16

Function Table:
Return  Name    Params
------  ----    ------
int     main
int     func2
int     func1

 ./parser demoGlobal.txt
Parsing successful

Global Symbol Table:

Type    ID      Value
----    --      ----
int     pear    14
int     apple   42

Function Table:
Return  Name    Params
------  ----    ------
int     main

 ./parser demoLocalScoping.txt
Parsing successful

Global Symbol Table:

Type    ID      Value
----    --      ----
int     apple   11

Function Table:
Return  Name    Params
------  ----    ------
int     main

 ./parser demoIf.txt
Parsing successful

Global Symbol Table:

Type    ID      Value
----    --      ----
int     apple   3

Function Table:
Return  Name    Params
------  ----    ------
int     main

 ./parser demoIf2.txt
Parsing successful

Global Symbol Table:

Type    ID      Value
----    --      ----
int     apple   1

Function Table:
Return  Name    Params
------  ----    ------
int     main
int     func    int xray

 ./parser demoWhile.txt
Parsing successful

Global Symbol Table:

Type    ID      Value
----    --      ----
int     pear    4

Function Table:
Return  Name    Params
------  ----    ------
int     main

 ./parser demoFor.txt
Parsing successful

Global Symbol Table:

Type    ID      Value
----    --      ----
int     x       5
int     i       0

Function Table:
Return  Name    Params
------  ----    ------

 ./parser demoFunc.txt
Parsing successful

Global Symbol Table:

Type    ID      Value
----    --      ----
int     apple   1

Function Table:
Return  Name    Params
------  ----    ------
int     main
int     func4
int     func3
int     func1
int     func2

 ./parser demoFuncReturn.txt
Parsing successful

Global Symbol Table:

Type    ID      Value
----    --      ----
int     apple   10

Function Table:
Return  Name    Params
------  ----    ------
int     main
int     func

 ./parser demoFuncCallFunc.txt
Parsing successful

Global Symbol Table:

Type    ID      Value
----    --      ----
int     apple   0

Function Table:
Return  Name    Params
------  ----    ------
int     main
int     func
int     func2   int aa

 ./parser demoFuncLocal.txt
Parsing successful

Global Symbol Table:

Type    ID      Value
----    --      ----
int     apple   1

Function Table:
Return  Name    Params
------  ----    ------
int     main
int     func1
int     func2

 ./parser demoParam.txt
Parsing successful

Global Symbol Table:

Type    ID      Value
----    --      ----

Function Table:
Return  Name    Params
------  ----    ------
int     func    const p, char c, int x

 ./parser demoFuncParam.txt
Parsing successful

Global Symbol Table:

Type    ID      Value
----    --      ----
int     apple   0

Function Table:
Return  Name    Params
------  ----    ------
int     main
int     funcc   int bb, int aa