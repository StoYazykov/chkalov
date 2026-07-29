/*
 * Copyright (C) 2025-2026  StoYazykov
 *
 * This file is part of Chkalov.
 *
 * Chkalov is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Chkalov is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Chkalov.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _AST_H
#define _AST_H

#include "token.h"

typedef enum {
    AST_STMT_BLOCK=0,
    AST_STMT_CALL,
    AST_EXPR_LITERAL,
    AST_BINARY,
    AST_EXPR_VARIABLE,
    AST_STMT_ASSIGN,
    AST_STMT_VAR_DECL,
    AST_EXPR_COMMA,
    AST_STMT_IF
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

typedef struct {
    AstNode base;
    char *name;
    uint8_t var_type;
} AstStmtVarDecl;

typedef struct {
    AstNode base;
    AstNode *left, *right;
} AstExprComma;

typedef struct {
    AstNode base, *cond;
    AstStmtBlock *body, *els;
} AstStmtIf;

typedef struct {
    AstNode base, *cond;
    AstStmtBlock *body;
} AstStmtWhile;

AstStmtBlock* ast_create_block();
AstStmtCall* ast_create_call(char *n, AstNode *arg);
AstExprLiteral* ast_create_literal(TokenType t, char *v);
AstBinary *ast_create_binary(TokenType t, AstNode *l, AstNode *r);
AstStmtAssign *ast_create_assign(char *n, AstNode *v);
AstStmtVarDecl *ast_create_vardecl(char *n, uint8_t vt);
AstExprComma *ast_create_comma(AstNode *l, AstNode *r);
AstExprVariable *ast_create_variable(char *n);
AstStmtIf *ast_create_if(AstNode *c, AstStmtBlock *b, AstStmtBlock *e);

AstNode *fold(AstNode *node);

void ast_free(AstNode *node);

void ast_add(AstNode *r, AstNode *n);

void ast_print(AstNode *node, int indent);

#endif
