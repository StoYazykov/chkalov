#ifndef _FUNCTION_H
#define _FUNCTION_H

#include "../../chkalov.h"
#include "../var_types/var_types.h"

typedef struct {
    char *name;
    size_t addr;
    Type ret_type;
} UserFunc;

void user_func_add(cv *funcs, char *name, size_t addr, Type ret_type);
UserFunc *user_func_find(cv *funcs, char *name);
void user_func_free(cv *funcs);

#endif
