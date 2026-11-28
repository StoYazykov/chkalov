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
    a->lastVar=0;
}

Variable *scopes_addVar(ScopeStack *a, ds varName, Type varType) {
    Variable var;
    cv *ptr;
    var.type=varType;
    var.name=strdup(varName);
    var.id=a->lastVar++;
    ptr=&((Scope *)cv_eptr(&a->scopes, a->scopes.s-1))->vars;
    cv_push(ptr, &var);
    return ((Variable *)cv_back(ptr));
}

Variable *scopes_getVar(ScopeStack *a, ds varName) {
    for(size_t i=a->scopes.s; i-->0;) {
        Scope *scope=(Scope *)cv_eptr(&a->scopes, i);
        for(size_t j=scope->vars.s; j-->0;) {
            Variable *var=(Variable *)cv_eptr(&scope->vars, j);
            if(!strcmp(var->name, varName)) return var;
        }
    }
    return NULL;
}

void scopes_enterScope(ScopeStack *a) {
    Scope scope;
    fprintf(stderr, "enterScope: scopes.s-1=%zu, scopes.s=%zu\n", a->scopes.s-1, a->scopes.s);
    cv_init(&scope.vars, 4, sz(Variable));
    cv_push(&a->scopes, &scope);
}

void scopes_exitScope(ScopeStack *a) {
    if(!a->scopes.s-1) return;
    Scope *scope=(Scope *)cv_eptr(&a->scopes, a->scopes.s-1);
    for(size_t i=0; i<scope->vars.s; i++) {
        Variable *var=(Variable *)cv_eptr(&scope->vars, i);
        free(var->name);
    }
    cv_free(&scope->vars);
    a->scopes.s--;
}

void scopes_free(ScopeStack *a) {
    Scope *scope;
    Variable *var;
    for(scope=(Scope *)a->scopes.d; scope<((Scope *)a->scopes.d)+a->scopes.s; scope++) {
        for(var=(Variable *)scope->vars.d; var<((Variable *)scope->vars.d)+scope->vars.s; var++) free(var->name);
        cv_free(&scope->vars);
    }
    cv_free(&a->scopes);
}
