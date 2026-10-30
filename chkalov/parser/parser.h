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

#include "../scope/scope.h"
#include "../ast/ast.h"
#include <ctype.h>
#include "../token/token.h"
#include "../lexer/lexer.h"
#include "../native/types.h"
#include "../function/function.h"
#include "../../chkalov.h"

typedef struct {
    size_t pos;
    char *name;
} InvokePatch;

typedef struct {
    int debug;
    cv code, file, funcs, libs, pool, invoke_patches;
    char *heap, *fn;
    size_t hp, hs, p, entry_point, current_base;
    AstStmtBlock *root;
    ScopeStack scopes;
} Parser;

#include "../sema/sema_check.h"

void par_init(Parser *a, const ds fni, const ds fno, int deb);
size_t par_heapIns(Parser *a, ds k);
ds par_mangle(Func *f);
void par_free(Parser *a);
Token* par_this(Parser *a);
Token* par_next(Parser *a);
uint8_t par_stt(ds s);
char *par_tts(uint8_t t);
char par_ttc(uint8_t t);
void par_split_commas(AstNode *n, cv *q);
Token* par_post(Parser *a);
void par_render(Parser *a, uint8_t op, int64_t v);
void par_parFile(Parser *a);
void expect(Parser *a, TokType t, char *s);
void skip(Parser *a, TokType t);
AstNode *par_par_primary(Parser *a);
AstNode *par_par_term(Parser *a);
AstNode *par_par_expr(Parser *a);
AstNode *par_par_comma(Parser *a, AstNode *l);
AstNode *par_par_comp(Parser *a);
size_t par_pool_add(Parser *a, char *str);
void par_optim(Parser *a);
AstStmtBlock *par_parBlock(Parser *a);

#endif
