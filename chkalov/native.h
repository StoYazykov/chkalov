#ifndef _NATIVE_H
#define _NATIVE_H

#include "types.h"
#include "parser.h"

void lib_add(Parser *a, ds n);
void func_add(Lib *a, Func z);
Func *func_find(Lib *a, Func z);

#endif
