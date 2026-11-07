#ifndef TYPES_H
#define TYPES_H

#include "../ast/ast.h"
#include "../../chkalov.h"
#include "../var_types/var_types.h"

typedef struct {
    ds name;
    cv args;
    Type ret_type;
} Func;

typedef struct {
    ds name;
    cv funcs;
} Lib;

#endif
