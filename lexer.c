// lexer.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

// global vars
char current;
char lookahead;
FILE* source_file;
int line = 1;
int isEOF = 0;
Token curr; // global var 

void openFile(char* filename) {
	source_file = fopen(filename, "r");
	if (!source_file) exit(1);
	current = fgetc(source_file);
	if (current == EOF) isEOF = 1; 
	else lookahead = fgetc(source_file);
}

void closeFile() {
	if (source_file) fclose(source_file);
}

void nextChar() {
	if (current == '\n') line++;
	current = lookahead;
	if (current == EOF) isEOF = 1;
	lookahead = fgetc(source_file);
}

void printToken(Token token) {
	const char* tokenNames[] = { "INTEGER", "FLOAT", "STRING", "CHAR", "COMMENT",
		"TYPE", "RESERVED", "OPERATOR", "IDENTIFIER" };
	printf("%s: %s", tokenNames[token.type], token.lexeme);
	if (token.type == TYPE_INTEGER) {
		printf(" (value: %d)", token.value);
	} else if (token.type == TYPE_FLOAT) {
		printf(" (value: %.2f)", token.fvalue);
	} printf("\n");
}

int isOperator(char c) {
	return (c == '+' || c == '-' || c == '*' || c == '/' ||
		c == '(' || c == ')' || c == '{' || c == '}' ||
		c == ';' || c == ',' || c == '=' || c == '<' || c == '>' ||
		c == '!' || c == '&' || c == '|' || c == '%');
}

int isReserved(char* word) {
	const char* reserve[] = {
   "break","case", "continue", "class", "catch",
   "do", "default", "def","else", "enum", "extends", "for",
   "false", "if", "import", "new", "private",
   "public", "protected", "return", "static", "struct", "switch",
   "super", "this", "try", "while", NULL };
	// check if curr word == reserve
	for (int i = 0; reserve[i] != NULL; i++) {
		if (strcmp(word, reserve[i]) == 0) {
			return 1;
		}
	} return 0;
}

int isType(char* word) {
	const char* types[] = { "boolean", "char", "const", "double", "float", "int",
		"long", "short", "void", "volatile", NULL };
	for (int i = 0; types[i] != NULL; i++) {
		if (strcmp(word, types[i]) == 0) {
			return 1;
		}
	} return 0;
}

Token nextToken() {
	Token token;
	token.line = line;
    memset(token.lexeme, 0, sizeof(token.lexeme));
	token.value = 0;
	token.fvalue = 0.0;
	if (isEOF) {
		token.type = TYPE_EOF;
		strcpy(token.lexeme, "EOF");
		return token;
	} // skip whitespace 
	while (!isEOF && (current == ' ' || current == '\t' || current == '\n' || current == '\r')) nextChar();
	if (isEOF) {
		token.type = TYPE_EOF;
		strcpy(token.lexeme, "EOF");
		return token;
	} // handle nums
	if (isdigit(current)) {
		int i = 0;
		int decimal = 0;
		while (!isEOF && (isdigit(current) || current == '.')) {
			if (current == '.') {
				if (decimal) break;
				decimal = 1;
			} token.lexeme[i++] = current; // handle lookahead
			nextChar();
		} token.lexeme[i] = '\0';
		if (decimal) {
			token.type = TYPE_FLOAT;
			token.fvalue = atof(token.lexeme);
		} else {
			token.type = TYPE_INTEGER;
			token.value = atoi(token.lexeme);
		} return token;
	} // handle string
	if (current == '"') {
		int i = 0;
		nextChar();
		while (current != '"' && !isEOF) {
			token.lexeme[i++] = current;
			nextChar();
		} if (isEOF) {
			token.type = TYPE_EOF;
			strcpy(token.lexeme, "EOF");
			return token;
		} nextChar();
		token.lexeme[i] = '\0';
		token.type = TYPE_STRING;
		return token;
	} // handle char literals
	if (current == '\'') {
		nextChar();
		if (current == EOF) {
			token.type = TYPE_EOF;
			strcpy(token.lexeme, "EOF");
			return token;
		} token.lexeme[0] = current;
		token.lexeme[1] = '\0';
		token.value = (int)current;
		nextChar();
		if (current != '\'') return nextToken();
		nextChar();
		token.type = TYPE_CHAR;
		return token;
	} // handle comment
	if (current == '/' && lookahead == '/') {
		while (!isEOF && current != '\n') nextChar();
		token.type = TYPE_COMMENT;
		strcpy(token.lexeme, "//...");
		return nextToken();
	} else if (current == '/' && lookahead == '*') {
		while (!(current == '*' && lookahead == '/') && !isEOF) {
			nextChar();
		} if(current == '*' && lookahead == '/') {
			nextChar();
			nextChar();
		} else {
			token.type = TYPE_EOF;
			strcpy(token.lexeme, "EOF");
			return token;
		}
		token.type = TYPE_COMMENT;
		strcpy(token.lexeme, "/*...*/");
		return nextToken();
	} // handle reserved/type/identifier
	if (isalpha(current) || current == '_') {
		int i = 0;
		while (!isEOF && (isalnum(current) || current == '_')) {
			token.lexeme[i++] = current;
			nextChar();
		} token.lexeme[i] = '\0';
		if (isType(token.lexeme)) token.type = TYPE_TYPE;
		else if (isReserved(token.lexeme)) token.type = TYPE_RESERVED;
		else token.type = TYPE_IDENTIFIER;
		return token;
	} // handle operator
	if (isOperator(current)) {
		token.type = TYPE_OPERATOR;
		memset(token.lexeme, 0, sizeof(token.lexeme));
		token.lexeme[0] = current;
		token.lexeme[1] = '\0';
		int twoCh = 0; // track if 2char op
		switch (current) {
		case '=':
			if (lookahead == '=') { strcpy(token.lexeme, "=="); twoCh = 1; } break;
		case '!':
			if (lookahead == '=') { strcpy(token.lexeme, "!=");	twoCh = 1; } break;
		case '+':
			if (lookahead == '=') { strcpy(token.lexeme, "+="); twoCh = 1; }
			else if (lookahead == '+') { strcpy(token.lexeme, "++"); twoCh = 1; } break;
		case '-':
			if (lookahead == '=') { strcpy(token.lexeme, "-="); twoCh = 1; }
			else if (lookahead == '-') { strcpy(token.lexeme, "--"); twoCh = 1; } break;
		case '>':
			if (lookahead == '=') { strcpy(token.lexeme, ">="); twoCh = 1; } break;
		case '<':
			if (lookahead == '=') {
				strcpy(token.lexeme, "<=");	twoCh = 1; } break;
		case '&':
			if (lookahead == '&') {
				strcpy(token.lexeme, "&&"); twoCh = 1; } break;
		case '|':
			if (lookahead == '|') {
				strcpy(token.lexeme, "||"); twoCh = 1; } break;
		case '%':
			if (lookahead == '=') { strcpy(token.lexeme,"%="); twoCh = 1; }
			else { strcpy(token.lexeme, "%"); } break;
		case '*':
			if (lookahead == '=') { strcpy(token.lexeme, "*="); twoCh = 1; }
			else { strcpy(token.lexeme, "*"); } break;
		case '/':
			if (lookahead == '=') { strcpy(token.lexeme, "/="); twoCh = 1; }
			else { strcpy(token.lexeme, "/"); } break;
		case ';':
			break;	
		default: // single op
			token.lexeme[0] = current;
			token.lexeme[1] = '\0';
			break;
		} if (twoCh) {
			nextChar();
		} nextChar();
		return token;
	} // if not match
	token.type = TYPE_EOF;
	strcpy(token.lexeme, "EOF");
	return token;
}