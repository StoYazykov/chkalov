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
#include "../chkalov.h"

typedef struct {
    cv funcs;
    ds name;
} Import;

bool imp_cont(Import *a, ds b);
void imp_add(Import *a, ds z);

typedef struct {
    ds fn;
    int debug;
    cv code, file;
    char *heap;
    size_t hp, hs;
    cv imports;
    AstStmtBlock *root;
    ScopeStack scopes;
    size_t p;
} Parser;

void par_init(Parser *a, const ds fni, const ds fno, int deb);
size_t par_heapIns(Parser *a, ds k);
void par_free(Parser *a);
Token* par_this(Parser *a);
Token* par_next(Parser *a);
uint8_t par_stt(ds s);
char *par_tts(uint8_t t);
Token* par_post(Parser *a);
void par_render(Parser *a, uint8_t op, uint8_t ty, int64_t v);
void par_parFile(Parser *a);
void expect(Parser *a, TokenType t, char *s);
AstNode *par_par_primary(Parser *a);
AstNode *par_par_term(Parser *a);
AstNode *par_par_expr(Parser *a);
AstNode *par_par_comma(Parser *a, AstNode *l);
AstNode *par_par_comp(Parser *a);
void par_optim(Parser *a);

#endif
