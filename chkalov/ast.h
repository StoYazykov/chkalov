#ifndef _AST_H
#define _AST_H

#include "token.h"

typedef enum {
    AST_STMT_BLOCK=0,
    AST_STMT_CALL,
    AST_EXPR_LITERAL,
    AST_BINARY,
    AST_EXPR_VARIABLE,
    AST_STMT_ASSIGN
} AstNodeType;

typedef struct {
    AstNodeType type;
} AstNode;

typedef struct {
    AstNode base;
    AstNode **stats;
    size_t count;
} AstStmtBlock;

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

typedef struct {
    AstNode base;
    unsigned char op;
    AstNode *left, *right;
} AstBinary;

typedef struct {
    AstNode base;
    char *name;
    AstNode *value;
} AstStmtAssign;

typedef struct {
    AstNode base;
    char *name;
} AstExprVariable;

AstStmtBlock* ast_create_block();
AstStmtCall* ast_create_call(char *n, AstNode *arg);
AstExprLiteral* ast_create_literal(TokenType t, char *v);
AstBinary *ast_create_binary(TokenType t, AstNode *l, AstNode *r);
AstNode *fold(AstNode *node);

void ast_free(AstNode *node);

void ast_add(AstNode *r, AstNode *n);

void ast_print(AstNode *node, int indent);

#endif
