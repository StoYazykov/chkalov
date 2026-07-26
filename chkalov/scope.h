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
