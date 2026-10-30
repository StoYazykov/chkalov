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

#include "../token/token.h"
#include "../../c-vectors/cv.h"

typedef enum {
    AST_STMT_BLOCK=0,
    AST_STMT_CALL,
    AST_EXPR_LITERAL,
    AST_BINARY,
    AST_EXPR_VARIABLE,
    AST_STMT_ASSIGN,
    AST_STMT_VAR_DECL,
    AST_EXPR_COMMA,
    AST_STMT_IF,
    AST_STMT_WHILE,
    AST_STMT_FUN,
    AST_STMT_RET
} AstNodeType;

typedef struct {
    char *name;
    uint8_t type;
} Arg;

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
    cv args; // AstNode *
} AstStmtCall;

typedef struct {
    AstNode base;
    TokType lit_type;
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
    uint8_t type;
    size_t id;
} AstStmtAssign;

typedef struct {
    AstNode base;
    char *name;
    size_t id;
    uint8_t type;
} AstExprVariable;

typedef struct {
    AstNode base;
    char *name;
    uint8_t var_type;
    size_t id;
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

typedef struct {
    AstNode base;
    char *name;
    cv args; // Arg
    size_t base_id;
    uint8_t ret_type;
    AstStmtBlock *body;
} AstStmtFun;

typedef struct {
    AstNode base, *value;
} AstStmtRet;

AstStmtBlock* ast_create_block();
AstStmtCall* ast_create_call(char *n, cv args);
AstExprLiteral* ast_create_literal(TokType t, char *v);
AstBinary *ast_create_binary(TokType t, AstNode *l, AstNode *r);
AstStmtAssign *ast_create_assign(char *n, AstNode *v, size_t id, uint8_t type);
AstStmtVarDecl *ast_create_vardecl(char *n, uint8_t vt, size_t id);
AstExprComma *ast_create_comma(AstNode *l, AstNode *r);
AstExprVariable *ast_create_variable(char *n, size_t id, uint8_t type);
AstStmtIf *ast_create_if(AstNode *c, AstStmtBlock *b, AstStmtBlock *e);
AstStmtWhile *ast_create_while(AstNode *c, AstStmtBlock *b);
AstStmtFun *ast_create_fun(char *n, cv args, uint8_t ret_type, AstStmtBlock *b);
AstStmtRet *ast_create_ret(AstNode *v);

AstNode *fold(AstNode *node);

void ast_free(AstNode *node);

void ast_add(AstNode *r, AstNode *n);

void ast_print(AstNode *node, int indent);

#endif
