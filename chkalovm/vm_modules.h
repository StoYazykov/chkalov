#ifndef _VM_MODULES_H
#define _VM_MODULES_H

#include "../chkalov.h"
#include "../chkapi.h"

typedef struct Module {
    char *name;
    char *path;
    Vm *code;
    size_t code_size;
    uint32_t entry_point;
    PoolEntry *pool;
    size_t pool_size;
    struct {
        struct Module *module;
        Vm *entry;
        bool is_native;
        void *native_func;
    } *imports;
    size_t imports_count;
} Module;

typedef struct {
    Module *main_module;
    Module **modules;
    size_t modules_count;
    Module *current_module;
    char *heap;
    size_t heap_ptr, heap_size;
} VMState;

typedef struct {
    size_t vars_s, args_s;
    Vm *ret_addr;
    Module *module;
} Frame;

typedef Int (*CFUNC)(size_t argc, int64_t *argp, ChkEnv *env);

Module *load_module_path(VMState *vm, const char *path, const char *name);
Module *load_module(VMState *vm, const char *name);
void resolve_main(VMState *vm, Module *m);
Vm *find_export(Module *m, const char *name);

#endif
