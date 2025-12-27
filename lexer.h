#ifndef LEXER_H
#define LEXER_H

// tokens
typedef enum {
    TYPE_INTEGER,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_CHAR,
    TYPE_COMMENT,
    TYPE_TYPE,
    TYPE_RESERVED,
    TYPE_OPERATOR,
    TYPE_IDENTIFIER,
    TYPE_EOF,
} TokenType;

// struct
typedef struct {
    TokenType type;
    char lexeme[256];
    int value;
    float fvalue;
    int line;
} Token;

// global var
extern Token curr;

// func dec
void openFile(char* filename);
void closeFile();
void nextChar ();
void printToken(Token token);
int isOperator(char c);
int isReserved(char* word);
int isType(char* word);
Token nextToken();


#endif