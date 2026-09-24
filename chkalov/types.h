#ifndef TYPES_H
#define TYPES_H

#include "ast.h"
#include "../chkalov.h"

typedef struct {
    ds name;
    cv args;
    uint8_t ret_type;
} Func;

typedef struct {
    ds name;
    cv funcs;
} Lib;

#endif
