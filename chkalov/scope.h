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

#ifndef _SCOPE_H
#define _SCOPE_H

#include "../chkalov.h"

typedef struct {
    ds name;
    size_t id;
    unsigned char type;
} variable;

typedef struct {
    cv vars;
} Scope;

typedef struct {
    cv scopes;
    size_t gid;
} ScopeStack;

void scos_init(ScopeStack *a);
void scos_addv(ScopeStack *a, ds vn, unsigned char vt);
void scos_getv(ScopeStack *a, ds vn, variable *b);
void scos_es(ScopeStack *a);
void scos_ex(ScopeStack *a);
size_t scos_ggi(ScopeStack *a);
size_t scos_size(ScopeStack *a);
void scos_free(ScopeStack *a);

#endif
