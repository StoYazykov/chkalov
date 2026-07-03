#ifndef _SCOPE_H
#define _SCOPE_H

#include "../chkalov.h"

typedef struct {
    ds name;
    size_t id;
    unsigned char type;
} variable;

void var_init(variable *a, ds name, size_t id, unsigned char t);
void var_free(variable *a);

typedef struct {
    cv vars;
} Scope;

void sco_init(Scope *a);
void sco_addv(Scope *a, ds vn, unsigned char vt);
void sco_gv(Scope *a, ds vn, variable *b);
size_t sco_ggi();
void sco_free(Scope *a);

typedef struct {
    cv scopes;
} ScopeStack;

void scos_init(ScopeStack *a);
void scos_addv(ScopeStack *a, ds vn, unsigned char vt);
void scos_gv(ScopeStack *a, ds vn, variable *b);
void scos_es(ScopeStack *a);
void scos_ex(ScopeStack *a);
size_t scos_size(ScopeStack *a);
void scos_free(ScopeStack *a);


#endif
