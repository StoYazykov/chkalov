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

#ifndef PARSER_H
#define PARSER_H

#include "scope.h"
#include "ast.h"
#include "token.h"
#include <ctype.h>
#include "lexer.h"
#include "../chkalov.h"

typedef struct {
    ds name;
    cv args;
    uint8_t ret_type;
} Func;

typedef struct {
    ds name;
    cv funcs;
} Lib;

typedef struct {
    ds fn;
    int debug;
    cv code, file;
    char *heap;
    size_t hp, hs;
    cv libs;
    AstStmtBlock *root;
    ScopeStack scopes;
    size_t p;
} Parser;

void lib_add(Parser *a, ds n);
void func_add(Lib *a, Func z);
Func *func_find(Lib *a, Func z);

void par_init(Parser *a, const ds fni, const ds fno, int deb);
size_t par_heapIns(Parser *a, ds k);
ds par_mangle(Func *f);
void par_free(Parser *a);
Token* par_this(Parser *a);
Token* par_next(Parser *a);
uint8_t par_stt(ds s);
char *par_tts(uint8_t t);
void par_split_commas(AstNode *n, cv *q);
Token* par_post(Parser *a);
void par_render(Parser *a, uint8_t op, int64_t v);
void par_parFile(Parser *a);
void expect(Parser *a, TokType t, char *s);
void skip(Parser *a, TokType *t);
AstNode *par_par_primary(Parser *a);
AstNode *par_par_term(Parser *a);
AstNode *par_par_expr(Parser *a);
AstNode *par_par_comma(Parser *a, AstNode *l);
AstNode *par_par_comp(Parser *a);
void par_optim(Parser *a);
AstStmtBlock *par_parBlock(Parser *a);

#endif
