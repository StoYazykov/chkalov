#ifndef _TOKEN_H
#define _TOKEN_H

#include <string>

using namespace std;

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

class Token {
public:
    TokenType type;
    string value;
    Token(TokenType __type = NOT, string __value = "");
    bool eat(Token token);
};

#endif
