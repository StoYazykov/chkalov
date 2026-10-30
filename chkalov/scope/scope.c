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

#include "scope.h"

void scopes_init(ScopeStack *a) {
    cv_init(&a->scopes, 4, sz(Scope));
    a->lastVar=a->curScope=0;
}

Variable *scopes_addVar(ScopeStack *a, ds varName, uint8_t varType) {
    Variable var;
    cv *ptr;
    var.type=varType;
    var.name=strdup(varName);
    var.id=a->lastVar++;
    ptr=&((Scope *)cv_eptr(&a->scopes, a->curScope))->vars;
    cv_push(ptr, &var);
    return ((Variable *)cv_back(ptr));
}

Variable *scopes_getVar(ScopeStack *a, ds varName) {
    Scope *scope;
    Variable *var;
    for(scope=(((Scope *)a->scopes.d)+a->curScope); scope>=(Scope *)a->scopes.d; scope--) {
        for(var=(((Variable *)scope->vars.d)+scope->vars.s-1); var>=(Variable *)scope->vars.d; var--) {
            if(!strcmp(var->name, varName)) return var;
        }
    }
    return NULL;
}

void scopes_enterScope(ScopeStack *a) {
    Scope scope;
    printf("enterScope: curScope=%zu, scopes.s=%zu\n", a->curScope, a->scopes.s);
    cv_init(&scope.vars, 4, sz(Variable));
    cv_push(&a->scopes, &scope);
    a->curScope=a->scopes.s-1;
}

void scopes_exitScope(ScopeStack *a) {
    a->curScope--;
}

void scopes_free(ScopeStack *a);
