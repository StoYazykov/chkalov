#include "../chkapi.h"

Slot print(size_t argc, Slot *argp, ChkEnv *env) {
    if(ISSTR(argp->type)) printf("%s", *env->_heap+argp->value);
    else printf("%d", argp->value);
    return (Slot){0x00};
}

Slot println(size_t argc, Slot *argp, ChkEnv *env) {
    if(ISSTR(argp->type)) puts(*env->_heap+argp->value);
    else printf("%d\r\n", argp->value);
    return (Slot){0x00};
}

Slot readln(size_t argc, Slot *argp, ChkEnv *env) {
    char *mem=env->alloc(env, 256);
    fgets(mem, 256, stdin);
    return (Slot){STR, mem-(uintptr_t)*env->_heap};
}
