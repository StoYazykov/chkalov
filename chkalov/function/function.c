#include "function.h"

void user_func_add(cv *funcs, char *name, size_t addr, Type ret_type) {
    UserFunc uf={0};
    uf.name=strdup(name);
    uf.addr=addr;
    uf.ret_type=ret_type;
    cv_push(funcs, &uf);
}

UserFunc *user_func_find(cv *funcs, char *name) {
    size_t i;
    for(i=0; i<funcs->s; i++) {
        UserFunc *uf=cv_eptr(funcs, i);
        if(!strcmp(uf->name, name)) return uf;
    }
    return NULL;

}

void user_func_free(cv *funcs) {
    size_t i;
    for(i=0; i<funcs->s; i++) {
        UserFunc *uf=cv_eptr(funcs, i);
        free(uf->name);
    }
    cv_free(funcs);
}

void user_func_add_extern(cv *funcs, char *name, size_t F_id, Type ret_type) {
    UserFunc uf;
    uf.name=strdup(name);
    uf.addr=0;
    uf.ret_type=ret_type;
    uf.is_extern=true;
    uf.F_id=F_id;
    cv_push(funcs, &uf);
}
