#include "native.h"

void lib_add(Parser *a, ds n) {
    Lib b;
    cv_init(&b.funcs, 16, sizeof(Func));
    b.name=strdup(n);
    cv_push(&a->libs, &b);
}

void func_add(Lib *a, Func z) {
    Func *copy=malloc(sizeof(Func));
    copy->name=strdup(z.name);
    copy->ret_type=z.ret_type;
    cv_copy(&copy->args, &z.args);
    cv_push(&a->funcs, copy);
}

Func *func_find(Lib *a, Func z) {
    Func *t;
    size_t i;
    for(i=0; i<a->funcs.s; i++) {
        t=cv_eptr(&a->funcs, i);
        if(!strcmp(t->name, z.name)&&cv_equ(&t->args, &z.args)) return t;
    }
    return NULL;
}
