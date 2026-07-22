#ifndef _AST_H
#define _AST_H

#include "token.h"

typedef enum {
    AST_PROGRAM,
    AST_STMT_CALL,
    AST_EXPR_LITERAL
} AstNodeType;

typedef struct {
    AstNodeType type;
} AstNode;

typedef struct {
    AstNode base;
    AstNode **stats;
    size_t count;
} AstProgram;

typedef struct {
    AstNode base;
    char *name;
    AstNode *arg;
} AstStmtCall;

typedef struct {
    AstNode base;
    TokenType type;
    char *value;
} AstExprLiteral;

AstProgram* ast_create_program();
AstStmtCall* ast_create_call(char *n, AstNode *arg);
AstExprLiteral* ast_create_literal(TokenType t, char *v);

void ast_free(AstNode *node);

void ast_add(AstNode *r, AstNode *n);

void ast_print(AstNode *node, int indent);

#endif
