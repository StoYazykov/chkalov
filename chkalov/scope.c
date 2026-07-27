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

void scos_init(ScopeStack *a) {
    cv_init(&a->scopes, 4, sz(Scope));
    a->gid=0;
}
void scos_addv(ScopeStack *a, ds vn, unsigned char vt) {
    printf("scos_addv: '%s' (len=%zu)\n", vn, strlen(vn));
    Scope *q=cv_eptr(&a->scopes, a->scopes.s-1);
    variable b;
    b.name=NULL;
    b.name=strdup(vn);
    b.type=vt;
    b.id=a->gid++;
    cv_push(&q->vars, &b);
}
void scos_getv(ScopeStack *a, ds vn, variable *b) {
    printf("scos_getv: searching '%s' (len=%zu)\n", vn, strlen(vn));
    Scope *ap;
    ssize_t i;
    b->name=NULL;
    printf("a->scopes.s: %llu\r\n", a->scopes.s);
    for(i=a->scopes.s-1; i>=0; i--) {
        printf("Iteration %llu\r\n", i);
        ap=(Scope *)cv_eptr(&a->scopes, i);
        variable *p;
        size_t j;
        for(j=0; j<ap->vars.s; j++) {
            p=(variable *)cv_eptr(&ap->vars, j);
            if(!strcmp(p->name, vn)) {
                b->name=p->name;
                b->type=p->type;
                b->id=p->id;
                printf("sco_getv Found! b->name: \'%s\', b->id: %llu!\r\n", b->name, b->id);
                return;
            }
        }
    }
}
void scos_es(ScopeStack *a) {
    Scope oa;
    cv_init(&oa.vars, 4, sz(variable));
    cv_push(&a->scopes, &oa);
}
void scos_ex(ScopeStack *a) {
    Scope *v;
    size_t j;
    variable *f;
    v=(Scope *)cv_eptr(&a->scopes, a->scopes.s-1);
    for(j=0; j<v->vars.s; j++) {
        f=(variable*)cv_eptr(&v->vars, j);
        free(f->name);
    }
    cv_free(&v->vars);
    cv_popr(&a->scopes);
}
size_t scos_ggi(ScopeStack *a) { return a->gid; }
size_t scos_size(ScopeStack *a) { return a->scopes.s; }
void scos_free(ScopeStack *a) {
    while(a->scopes.s>0) {
        scos_ex(a);
    }
    cv_free(&a->scopes);
}
