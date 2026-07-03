#include "scope.h"

static variable nullvar;
static size_t globalId=0;

void var_init(variable *a, ds na, size_t i, unsigned char t) {
    a->name=NULL;
    ds_str(&(a->name), na);
    a->id=i;
    a->type=t;
}
void var_free(variable *a) {
    free(a->name);
}
void sco_init(Scope *a) {
    cv_init(&a->vars, 4, sz(variable));
}
void sco_addv(Scope *a, ds vn, unsigned char vt) {
    variable b;
    b.name = NULL;
    ds_str(&b.name, vn);
    b.type=vt;
    b.id=globalId++;
    cv_push(&a->vars, &b);
}
void sco_gv(Scope *a, ds vn, variable *b) {
    variable *p;
    for(size_t i=0; i<a->vars.s; i++) {
        p=(variable *)cv_eptr(&a->vars, i);
        if(!strcmp(p->name, vn)) {
            b->name=strdup(p->name);
            return;
        }
    }
    b->name=NULL;
}
size_t sco_ggi() {
    return globalId?globalId-1:0;
}
void sco_free(Scope *a) {
    for(size_t i=0; i<a->vars.s; i++) {
        variable *v=(variable*)cv_eptr(&a->vars, i);
        free(v->name);
    }
    cv_free(&a->vars);
}
void scos_init(ScopeStack *a) {
    cv_init(&a->scopes, 4, sz(Scope));
}
void scos_addv(ScopeStack *a, ds vn, unsigned char vt) {
    Scope *q=cv_eptr(&a->scopes, a->scopes.s-1);
    sco_addv(q, vn, vt);
}
void scos_gv(ScopeStack *a, ds vn, variable *b) {
    Scope *ap;
    b->name=NULL;
    for(size_t i=a->scopes.s; i>0; --i) {
        ap=(Scope *)cv_eptr(&a->scopes, i);
        sco_gv(ap, vn, b);
        if(b->name) return;
    }
}
void scos_es(ScopeStack *a) {
    Scope oa;
    sco_init(&oa);
    cv_push(&a->scopes, &oa);
}
void scos_ex(ScopeStack *a) {
    sco_free(cv_eptr(&a->scopes, a->scopes.s-1));
    cv_popr(&a->scopes);
}
size_t scos_size(ScopeStack *a) { return a->scopes.s; }
void scos_free(Scope *a) {
    for(size_t i=0; i<a->vars.s; i++) {
        Scope *v=(Scope *)cv_eptr(&a->scopes, i);
        sco_free(v);
    }
    cv_free(&a->scopes);
}
