// parser.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

typedef struct Symbol {
    char name[256];
    char type[20];
    int intVal; 
    float floatVal;
    char charVal;
    struct Symbol* next;
} Symbol;

typedef struct Function {
    char name[256];
    char returnType[20];
    struct Symbol* params;
    struct Symbol* locals;
    struct Function* next;
} Function;

// return vals
typedef struct StackFrame {
    struct Symbol* locals;
    int returnValue;
    struct StackFrame* prev;
} StackFrame;

// global table
Symbol* table = NULL;
Function* funcTable = NULL;
Function* currentFunc = NULL;
StackFrame* currentFrame = NULL;
int returnValue = 0;
int inFunc = 0;

// func dec
int program();
int statement();
int declaration();
int expression();
void printTable();
void addSymbol(char* name, char* type, int val);
void syntaxError(char* msg);
Symbol* findSymbol(char* name);

// operator precedence
int logicalOr();
int logicalAnd(); 
int equality();
int comparison();
int additive();
int multiplicative();
int unary();
int primary();
// func prototypes
int block();
int ifStat();
int whileStat();
int forStat();
int returnStat();
int funcCall(char* name);
void addFunc(char*name, char* returnType);
Function* findFunc(char* name);

int main(int argc, char* argv[]) {
    if (argc != 2) return 1;
    openFile(argv[1]);
    curr = nextToken(); // get first
    if (program()) {
        printf("Parsing successful\n");
        printTable();
    } else {
        printf("Parsing failed\n");
    } closeFile();
    return 0;
}

// func implementation
int program() {
     while (1) {
        if (curr.type == TYPE_EOF) return 1;
        if (!statement()) return 0;
    } return 1;
}

int statement() {
     if (curr.type == TYPE_TYPE) {
        return declaration();
    } else if (curr.type == TYPE_IDENTIFIER) {
        char name[256];
        strcpy(name, curr.lexeme);
        curr = nextToken();
        if (curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, "(") == 0) {
            Function* func = funcTable;
            while (func != NULL && strcmp(func->name, name) != 0) func = func->next;
            if (func == NULL) {
                syntaxError("Undefined function");
                return 0;
            } curr = nextToken();
            Symbol* param = func->params;
            while (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, ")") != 0) {
                int argValue = expression();
                if (param != NULL) {
                    if (strcmp(param->type, "int") == 0) param->intVal = argValue;
                    else if (strcmp(param->type, "float") == 0) param->floatVal = (float)argValue;
                    else if (strcmp(param->type, "char") == 0) param->charVal = (char)argValue;
                    param = param->next;
                } if (curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, ",") == 0) curr = nextToken();
            } if (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, ")") != 0) return 0;
            curr = nextToken();
            returnValue = 0;
            if (curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, ";") == 0) curr = nextToken();
            return 1;
        } else {
            if (curr.type != TYPE_OPERATOR ||
                (strcmp(curr.lexeme, "=") != 0 &&
                strcmp(curr.lexeme, "+=") != 0 &&
                strcmp(curr.lexeme, "-=") != 0 && 
                strcmp(curr.lexeme, "*=") != 0 &&
                strcmp(curr.lexeme, "/=") != 0 &&
                strcmp(curr.lexeme, "%=") != 0)) {
                    syntaxError("Expected assignment operator");
                    return 0;
            } char op[3];
            strcpy(op, curr.lexeme);
            curr = nextToken();
            int value = expression();
            Symbol* sym = findSymbol(name);
            if (!sym) {
                syntaxError("Variable not declared");
                return 0;
            } int current = 0;
            if (strcmp(sym->type, "int") == 0) current = sym->intVal;
            else if (strcmp(sym->type, "float") == 0) current = (int)sym->floatVal;
            else if (strcmp(sym->type, "char") == 0) current = (char)sym->charVal;
            if (strcmp(op, "=") == 0) {
                if (strcmp(sym->type, "int") == 0) sym->intVal = value;
                else if (strcmp(sym->type, "float") == 0) sym->floatVal = (float)value;
                else if (strcmp(sym->type, "char") == 0) sym->charVal = (char)value;
            } else if (strcmp(op, "+=") == 0) {
                if (strcmp(sym->type, "int") == 0) sym->intVal = current + value;
                else if (strcmp(sym->type, "float") == 0) sym->floatVal = current + value;
                else if (strcmp(sym->type, "char") == 0) sym->charVal = current + value;
            } else if (strcmp(op, "-=") == 0) {
                if (strcmp(sym->type, "int") == 0) sym->intVal = current - value;
                else if (strcmp(sym->type, "float") == 0) sym->floatVal = current - value;
                else if (strcmp(sym->type, "char") == 0) sym->charVal = current - value;
            } else if (strcmp(op, "*=") == 0) {
                if (strcmp(sym->type, "int") == 0) sym->intVal = current * value;
                else if (strcmp(sym->type, "float") == 0) sym->floatVal = current * value;
                else if (strcmp(sym->type, "char") == 0) sym->charVal = current * value;
            } else if (strcmp(op, "/=") == 0) {
                if (value == 0) {
                    syntaxError("Divide by zero");
                    return 0;
                } if (strcmp(sym->type, "int") == 0) sym->intVal = current / value;
                else if (strcmp(sym->type, "float") == 0) sym->floatVal = current / value;
                else if (strcmp(sym->type, "char") == 0) sym->charVal = current / value;
            } else if (strcmp(op, "%=") == 0) {
                sym->intVal = current % value;
            } if (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, ";") != 0) {
                syntaxError("Expected ';'");
                return 0;
            } curr = nextToken();
            return 1;
        }
    } else if (curr.type == TYPE_RESERVED) {
        if (strcmp(curr.lexeme, "if") == 0) return ifStat();
        else if (strcmp(curr.lexeme, "while") == 0) return whileStat();
        else if (strcmp(curr.lexeme, "for") == 0) return forStat();
        else if (strcmp(curr.lexeme, "return") == 0) return returnStat();
    } else if (curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, "{") == 0) {
        return block();
    } else if (curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, ";") == 0) {
        curr = nextToken();
        return 1;
    } else {
        syntaxError("Expected declaration, assignment, or statement");
        return 0;
    } return 0;
}

 int declaration() {
    char type[20], name[256];
    int value = 0;
    if (curr.type != TYPE_TYPE) { 
        syntaxError("Expected type"); 
        return 0; 
    } strcpy(type, curr.lexeme);
    curr = nextToken(); 
    if (curr.type != TYPE_IDENTIFIER) { 
        syntaxError("Expected variable name"); 
        return 0; 
    } strcpy(name, curr.lexeme);
    curr = nextToken();
    if (curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, "(") == 0) {
        curr = nextToken(); 
        // parse parameters
        Symbol* params = NULL;
        while (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, ")") != 0) {
            if (curr.type == TYPE_TYPE) {
                char paramType[20], paramName[256];
                strcpy(paramType, curr.lexeme);
                curr = nextToken();
                if (curr.type != TYPE_IDENTIFIER) {
                    syntaxError("Expected parameter name");
                    return 0;
                } strcpy(paramName, curr.lexeme);
                Symbol* param = malloc(sizeof(Symbol));
                strcpy(param->name, paramName);
                strcpy(param->type, paramType);
                param->next = params;
                params = param;
                curr = nextToken();
                if (curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, ",") == 0) 
                    curr = nextToken();
            } else { break; }
        } if (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, ")") != 0) {
            syntaxError("Expected ')'");
            return 0;
        } curr = nextToken();
        // add to function table
        Function* func = malloc(sizeof(Function));
        strcpy(func->name, name);
        strcpy(func->returnType, type);
        func->params = params;
        func->locals = NULL;
        func->next = funcTable;
        funcTable = func;
        currentFunc = func;
        inFunc = 1;
        int result = block();
        inFunc = 0;
        currentFunc = NULL;
        return result;
    } // variable declaration
    if (curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, "=") == 0) {
        curr = nextToken();
        value = expression();
    } if (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, ";") != 0) {
        syntaxError("Expected ';'");
        return 0;
    } curr = nextToken();
    // add to symbol table
    if (inFunc) {
        if (currentFunc == NULL) { 
            syntaxError("No active func"); 
            return 0; 
        } Symbol* local = malloc(sizeof(Symbol));
        strcpy(local->name, name);
        strcpy(local->type, type);
        if (strcmp(type, "int") == 0) local->intVal = value;
        else if (strcmp(type, "float") == 0) local->floatVal = (float)value;
        else if (strcmp(type, "char") == 0) local->charVal = (char)value;
        local->next = currentFunc->locals;
        currentFunc->locals = local;
    } else { 
        addSymbol(name, type, value); 
    } return 1;
}

int expression() {
    if (curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, ";") == 0) return 0;
    return logicalOr();
}

void printTable() {
    printf("\nGlobal Symbol Table:\n");
    printf("\nType\tID\tValue\n");
    printf("----\t--\t----\n");
    Symbol* cur = table;
    while (cur != NULL) {
        if (strcmp(cur->type, "int") == 0) {
            printf("%s\t%s\t%d\n", cur->type, cur->name, cur->intVal);
        } else if (strcmp(cur->type, "float") == 0) {
            printf("%s\t%s\t%.2f\n", cur->type, cur->name, cur->floatVal);
        } else if (strcmp(cur->type, "char") == 0) {
            printf("%s\t%s\t%c\n", cur->type, cur->name, cur->charVal);
        } cur = cur->next;
    } printf("\nFunction Table:\n");
    printf("Return\tName\tParams\n");
    printf("------\t----\t------\n");
    Function* func = funcTable;
    while (func != NULL) {
        printf("%s\t%s\t", func->returnType, func->name);
        Symbol* param = func->params;
        while (param != NULL) {
            printf("%s %s", param->type, param->name);
            param = param->next;
            if (param != NULL) printf(", ");
        } printf("\n");
        func = func->next;
    }
}

void addSymbol(char* name, char* type, int val) {
    Symbol* sym = malloc(sizeof(Symbol));
    strcpy(sym->name, name);
    strcpy(sym->type, type);
    if (strcmp(type, "int") == 0) {
        sym->intVal = val;
    } else if (strcmp(type, "float") == 0) {
        sym->floatVal = (float)val;
    } else if (strcmp(type, "char") == 0) {
        sym->charVal = (char)val;
    } sym->next = table;
    table = sym;
}

void syntaxError(char* msg) {
    printf("Error at line %d:%s\n", curr.line, msg);
}

Symbol* findSymbol(char* name) {
    if (inFunc && currentFunc != NULL) {
        Symbol* local = currentFunc->locals;
        while(local != NULL) {
            if (strcmp(local->name, name) == 0) return local;
            local = local->next; 
        } // check func parameters
        Symbol* param = currentFunc->params;
        while (param != NULL) {
            if (strcmp(param->name, name) == 0) return param;
            param = param->next;
        }
    } // global variables
    Symbol* cur = table;
    while (cur != NULL) {
        if (strcmp(cur->name, name) == 0) return cur;
        cur = cur->next;
    } return NULL;
}

// expression parsing
int logicalOr() {
    int l = logicalAnd();   
    while (curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, "||") == 0) {
        curr = nextToken();
        int r = logicalAnd();
        l = l || r;
        if (curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, ";") == 0) {
            break;
        }
    } return l;
}

int logicalAnd() {
    int l = equality();
    while (curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, "&&") == 0) {
        curr = nextToken();
        int r = equality();
        l = l && r;
    } return l;
}

int equality() {
    int l = comparison();
    while (curr.type == TYPE_OPERATOR && (strcmp(curr.lexeme, "==") == 0 || strcmp(curr.lexeme, "!=") == 0)) {
        char* op = curr.lexeme;
        curr = nextToken();
        int r = comparison();
        if (strcmp(op, "==") == 0) {  l = l == r ? 1 : 0; } 
        else  { l = l != r ? 1 : 0; } 
    } return l;
}

int comparison() {
    int l = additive();
    while (curr.type == TYPE_OPERATOR && 
        (strcmp(curr.lexeme, "<") == 0 || strcmp(curr.lexeme, ">") == 0 || 
        strcmp(curr.lexeme, "<=") == 0 || strcmp(curr.lexeme, ">=") == 0))  {
        char*op = curr.lexeme;
        curr = nextToken();
        int r = additive();
        if (strcmp(op, "<") == 0) { l = (l < r) ? 1 : 0; }
        else if (strcmp(op, ">") == 0) { l = (l > r) ? 1 : 0; }
        else if (strcmp(op, "<=") == 0) { l = (l <= r) ? 1 : 0; }
        else { l = (l >= r) ? 1 : 0; }
    } return l;
}

int additive() {
    int l = multiplicative();
    while (curr.type == TYPE_OPERATOR && (strcmp(curr.lexeme, "+") == 0 || strcmp(curr.lexeme, "-") == 0)) {
        char op[3];
        strcpy(op, curr.lexeme);
        curr = nextToken();
        int r = multiplicative();
        if (strcmp(op, "+") == 0) { l = l + r; } 
        else { l = l - r; } 
    } return l;
}

int multiplicative() {
    int l = unary();
    while (curr.type == TYPE_OPERATOR && 
      (strcmp(curr.lexeme, "*") == 0 || strcmp(curr.lexeme, "/") == 0 || 
       strcmp(curr.lexeme, "%") == 0)) {
        char op[3];
        strcpy(op, curr.lexeme);
        curr = nextToken();
        int r = unary();
        if (strcmp(op, "*") == 0) { l = l * r; } 
        else if (strcmp(op, "/") == 0) {
            if (r == 0) {
                syntaxError("Division by zero");
                return 0;
            } l = l / r;
        } else { l = l % r; }      
    } return l; 
} 

int unary() {
    if (curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, "!") == 0) {
        curr = nextToken();
        return !unary();
    } else if (curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, "-") == 0) {
        curr = nextToken();
        return -unary();
    } else if (curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, "++") == 0) {
        curr = nextToken();
        return unary();
    } else if (curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, "--") == 0) {
        curr = nextToken();
        return unary();
    } return primary();
}

int primary() {
    int value = 0;
    if (curr.type == TYPE_INTEGER) {
        value = curr.value;
        curr = nextToken();
    } else if (curr.type == TYPE_FLOAT) {
        value = (int)curr.fvalue;
        curr = nextToken();
    } else if (curr.type == TYPE_CHAR) {
        value = curr.value;
        curr = nextToken();
    } else if (curr.type == TYPE_IDENTIFIER) {
        char identName[256];
        strcpy(identName, curr.lexeme);
        curr = nextToken();
        if (curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, "(") == 0) {
            Function* func = funcTable;
            while (func != NULL && strcmp(func->name, identName) != 0) func = func->next;
            if (func == NULL) {
                syntaxError("Undefined function");
                return 0;
            } curr = nextToken();
            Symbol* param = func->params;
            while (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, ")") != 0) {
                int argValue = expression();
                if (param != NULL) {
                    if (strcmp(param->type, "int") == 0) param->intVal = argValue;
                    else if (strcmp(param->type, "float") == 0) param->floatVal = (float)argValue;
                    else if (strcmp(param->type, "char") == 0) param->charVal = (char)argValue;
                    param = param->next;
                } if (curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, ",") == 0) curr = nextToken();
            } if (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, ")") != 0) {
                syntaxError("Expected ')' in function call");
                return 0;
            } curr = nextToken();
            value = returnValue;
        } else {
            Symbol* sym = findSymbol(identName);
            if (sym == NULL) value = 0;
            else {
                if (strcmp(sym->type, "int") == 0) value = sym->intVal;           
                else if (strcmp(sym->type, "float") == 0) value = (int)sym->floatVal;
                else if (strcmp(sym->type, "char") == 0) value = sym->charVal; 
            }
        }
    } else if (curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, "(") == 0) {
        curr = nextToken();
        if (curr.type == TYPE_EOF) { syntaxError("Unexpected EOF after '('"); return 0; }
        value = expression();
        if (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, ")") != 0) {
            syntaxError("Expected ')'");
            return 0;
        }  curr = nextToken();
    } else {
        return 0;
    } if (curr.type == TYPE_OPERATOR && (strcmp(curr.lexeme, "++") == 0 || strcmp(curr.lexeme, "--") == 0)) {
        curr = nextToken();
    } return value;
}

int block() {
    if (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, "{") != 0) return 0;
    curr = nextToken();
    while (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, "}") != 0) {
        if (curr.type == TYPE_EOF) { syntaxError("Unexpected EOF in block"); return 0; }
        if (!statement()) return 0;
    } curr = nextToken();
    return 1;
}

int ifStat() {
    if (curr.type != TYPE_RESERVED || strcmp(curr.lexeme, "if") != 0) return 0;
    curr = nextToken();
    if (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, "(") != 0) {
        syntaxError("Expected '(' after if");
        return 0;
    } curr = nextToken();
    int cond = expression(); 
    if (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, ")") != 0) {
        syntaxError("Expected ')' after if condition");
        return 0;
    } curr = nextToken();
    if (!statement()) return 0;
    // check for else
    if (curr.type == TYPE_RESERVED && strcmp(curr.lexeme, "else") == 0) {
        curr = nextToken();
        if (!statement()) return 0;
    } return 1;
}

int whileStat() {
    if (curr.type != TYPE_RESERVED || strcmp(curr.lexeme, "while") != 0) return 0;
    curr = nextToken();
    if (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, "(") != 0) {
        syntaxError("Expected '(' after while");
        return 0;
    } curr = nextToken();
    int cond = expression(); 
    if (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, ")") != 0) {
        syntaxError("Expected ')' after while condition");
        return 0;
    } curr = nextToken();
    return statement();
}

int forStat() {
    if (curr.type != TYPE_RESERVED || strcmp(curr.lexeme, "for") != 0) return 0;
    curr = nextToken();
    if (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, "(") != 0) {
        syntaxError("Expected '(' after for");
        return 0;
    } curr = nextToken();
    if (curr.type == TYPE_TYPE) {
        if (!statement()) return 0;
    } else if (curr.type == TYPE_IDENTIFIER) {
        expression();
        if (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, ";") != 0) {
            syntaxError("Expected ';' after initialization");
            return 0;
        } curr = nextToken();
    } else if (curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, ";") == 0) {
        curr = nextToken();
    } else {
        syntaxError("Invalid for loop initialization");
        return 0;
    } if (!(curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, ";") == 0)) expression();
    if (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, ";") != 0) {
        syntaxError("Expected ';' after for condition");
        return 0;
    } curr = nextToken();
    if (!(curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, ")") == 0)) expression();
    if (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, ")") != 0) {
        syntaxError("Expected ')' in for loop");
        return 0;
    } curr = nextToken();
    return statement();
}

int returnStat() {
    if (curr.type != TYPE_RESERVED || strcmp(curr.lexeme, "return") != 0) return 0;
    curr = nextToken();
    if (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, ";") != 0) returnValue = expression();
    if (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, ";") != 0) {
        syntaxError("Expected ';' after return");
        return 0;
    } curr = nextToken();
    return 1;
}

int funcCall(char* name) {
    Function* func = funcTable;
    while (func != NULL && strcmp(func->name, name) != 0) func = func->next;
    if (func == NULL) {
        syntaxError("Undefined function");
        return 0;
    } curr = nextToken();
    if (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, "(") != 0) return 0;
    curr = nextToken();
    int count = 0;
    while (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, ")") != 0) {
        expression();
        count++;
        if (curr.type == TYPE_OPERATOR && strcmp(curr.lexeme, ",") == 0) curr = nextToken();
    } if (curr.type != TYPE_OPERATOR || strcmp(curr.lexeme, ")") != 0) return 0;
    curr = nextToken();
    returnValue = 0;
    return 1;
}

void addFunc(char*name, char* returnType) {
    Function* func = malloc(sizeof(Function));
    strcpy(func->name, name);
    strcpy(func->returnType, returnType);
    func->params = NULL;
    func->locals = NULL;
    func->next = funcTable;
    funcTable = func;
}

Function* findFunc(char* name) {
    Function* func = funcTable;
    while (func != NULL) {
        if (strcmp(func->name, name) == 0) return func;
        func = func->next;
    } return NULL;
}