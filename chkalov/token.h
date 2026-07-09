#ifndef _TOKEN_H
#define _TOKEN_H

#include "../chkalov.h"

typedef enum {
    NOT,
	ENOF, NUMBER, STRING, LCALL, ID, CONVERT,
    VAR, FUN, IF, ELSE, WHILE, RETURN, IMPORT,
    PLUS, MINUS, STAR, SLASH, ASSIGN,
    EQ, NEQ, LT, BT, LE, BE,
    LPAREN, RPAREN, LBRACE, RBRACE,
    COMMA, DOT, COLON,
    CLASS
} TokenType;

typedef struct {
    TokenType type;
    ds value;
} Token;

void tok_init(Token *a, TokenType b, ds c);
void tok_free(Token *a);

#endif
