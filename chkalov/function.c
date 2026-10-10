#include "function.h"

void user_func_add(cv *funcs, char *name, size_t addr, uint8_t ret_type) {
    UserFunc uf;
    uf.name=strdup(name);
    uf.addr=addr;
    uf.ret_type=ret_type;
    cv_push(funcs, &uf);
}

size_t user_func_find(cv *funcs, char *name) {
    size_t i;
    for(i=0; i<funcs->s; i++) {
        UserFunc *uf=cv_eptr(funcs, i);
        if(!strcmp(uf->name, name)) return uf->addr;
    }
    return NONE;

}

void user_func_free(cv *funcs) {
    size_t i;
    for(i=0; i<funcs->s; i++) {
        UserFunc *uf=cv_eptr(&funcs, i);
        free(uf->name);
    }
    cv_free(funcs);
}
