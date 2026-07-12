#include "../chkapi.h"

Slot println(size_t argc, Slot *argp, ChkEnv *env) {
    if(ISSTR(argp->type)) puts(*env->_heap+argp->value);
    else printf("%d\r\n", argp->value);
    return (Slot){0x00};
}

Slot readln(size_t argc, Slot *argp, ChkEnv *env) {
    void *mem=env->alloc(env, 256);
    scanf("%s", mem);
    return (Slot){STR, *env->_heap-(char *)mem};
}
