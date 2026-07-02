#ifndef _TOKEN_H
#define _TOKEN_H

#include "../ds.h"

enum TokenType {
    NOT,
	ENOF, NUMBER, STRING, LCALL, ID, CONVERT,
    VAR, FUN, IF, ELSE, WHILE, RETURN, IMPORT,
    PLUS, MINUS, STAR, SLASH, ASSIGN,
    EQ, NEQ, LT, BT, LE, BE,
    LPAREN, RPAREN, LBRACE, RBRACE,
    COMMA, DOT, COLON, NEWLINE,
    TYPE
};

typedef struct {
public:
    TokenType type;
    ds value;
    Token(TokenType __type = NOT, string __value = "");
} Token;

#endif
