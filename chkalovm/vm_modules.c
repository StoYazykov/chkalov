#include "vm_modules.h"

static const int operand_sizes_base[]={
    1, 2, 4, 8, 8, 0, 0, 0, 8, 8, 8, 8, 0, 0, 8, 8, 8, 8
};

Vm *find_export(Module *m, const char *name) {
    for(size_t i=0; i<m->pool_size; i++) {
        if(m->pool[i].tag!='E') continue;
        char *ename=strdup(m->pool[i].name);
        char *paren=strchr(ename, '(');
        if(paren) *paren=0;
        if(!strcmp(ename, name)) {
            free(ename);
            return m->code+m->pool[i].operand;
        }
        free(ename);
    }
    return NULL;
}

Module *load_module_path(VMState *vm, const char *path, const char *name) {
    FILE *f=fopen(path, "rb");
    if(!f) error("Module not found: %s", path);

    Header hdr;
    fread(&hdr, 1, sz(Header), f);
    if(hdr.magic!=0x05020200) error("Bad magic: %s", path);

    Module *m=calloc(1, sizeof(Module));
    m->name=strdup(name);
    m->path=strdup(path);
    m->entry_point=hdr.entry_point;

    uint64_t base=vm->heap_ptr;
    if(hdr.heap_size>0) {
        vm->heap=realloc(vm->heap, vm->heap_size+hdr.heap_size);
        fread(vm->heap+base, 1, hdr.heap_size, f);
        vm->heap_ptr+=hdr.heap_size;
        vm->heap_size+=hdr.heap_size;
    }

    m->pool=calloc(hdr.pool_size, sizeof(PoolEntry));
    m->pool_size=hdr.pool_size;
    for(size_t i=0; i<hdr.pool_size; i++) {
        uint32_t len;
        fread(&len, 1, sz(uint32_t), f);
        char *buf=malloc(len);
        fread(buf, 1, len, f);
        char *p=buf;
        PoolEntry entry={0};
        entry.tag=*p++;
        switch(entry.tag) {
            case 'N':
            case 'L':
            case 'S':
                entry.name=strdup(p);
                break;
            case 'X':
            case 'F':
            case 'E':
                if(*p=='(') {
                    p++;
                    while(isxdigit((unsigned char)*p)) {
                        int d=(*p<='9')?*p-'0':(tolower(*p)-'a'+10);
                        entry.operand=entry.operand*16+d;
                        p++;
                    }
                    if(*p==')') p++;
                }
                if(*p) entry.name=strdup(p);
                break;
        }
        m->pool[i]=entry;
        free(buf);
    }

    uint8_t *raw=malloc(hdr.code_size);
    fread(raw, 1, hdr.code_size, f);
    m->code=calloc(hdr.code_size, sizeof(Vm));
    size_t i=0, ins=0;
    while(i<hdr.code_size) {
        Vm q={0};
        q.opcode=raw[i++];
        size_t size;
        if(q.opcode>=OP_CLOAD&&q.opcode<=OP_ASTORE) size=4;
        else if(q.opcode>=OP_CADD&&q.opcode<=OP_ANEG) size=0;
        else if(q.opcode==OP_CAST) size=1;
        else size=operand_sizes_base[q.opcode];
        memcpy(&q.value, raw+i, size);

        if(q.opcode==OP_APUSH) {
            q.value+=base;
        }

        m->code[ins++]=q;
        i+=size;
    }
    m->code_size=ins;
    free(raw);
    fclose(f);

    m->imports=calloc(hdr.pool_size, sizeof(*m->imports));
    m->imports_count=hdr.pool_size;

    vm->modules=realloc(vm->modules, (vm->modules_count+1)*sizeof(Module *));
    vm->modules[vm->modules_count++]=m;
    return m;
}

Module *load_module(VMState *vm, const char *name) {
    for(size_t i=0; i<vm->modules_count; i++)
        if(!strcmp(vm->modules[i]->name, name)) return vm->modules[i];

    char *base=getenv("CHKALOV");
    ds path=strdup(base);
    ds_cat(&path, "/lib/", name, ".cvm", NULL);

    Module *m=load_module_path(vm, path, name);
    free(path);

    for(size_t k=0; k<m->pool_size; k++) {
        if(m->pool[k].tag=='F') {
            size_t lib_id=m->pool[k].operand;
            Module *target=load_module(vm, m->pool[lib_id].name);
            char *fname=strdup(m->pool[k].name);
            char *paren=strchr(fname, '(');
            if(paren) *paren=0;
            Vm *entry=find_export(target, fname);
            if(!entry) error("Export not found: %s in %s", fname, target->name);
            m->imports[k].module=target;
            m->imports[k].entry=entry;
            m->imports[k].is_native=false;
            free(fname);
        } else if(m->pool[k].tag=='X') {
            size_t lib_id=m->pool[k].operand;
            char *libname=m->pool[lib_id].name;
            ds so_path=strdup(getenv("CHKALOV"));
            ds_cat(&so_path, "/lib/", libname, "/lib", libname, ".so", NULL);
            void *h=dlopen(so_path, RTLD_LAZY);
            free(so_path);
            if(!h) error("Cannot load: %s", dlerror());
            void *func=dlsym(h, m->pool[k].name);
            if(!func) { dlclose(h); error("Undefined: %s", m->pool[k].name); }
            m->imports[k].is_native=true;
            m->imports[k].native_func=func;
        }
    }
    return m;
}

void resolve_main(VMState *vm, Module *m) {
    for(size_t k=0; k<m->pool_size; k++) {
        if(m->pool[k].tag=='F') {
            size_t lib_id=m->pool[k].operand;
            Module *target=load_module(vm, m->pool[lib_id].name);
            char *fname=strdup(m->pool[k].name);
            char *paren=strchr(fname, '(');
            if(paren) *paren=0;
            Vm *entry=find_export(target, fname);
            if(!entry) error("Export not found: %s in %s", fname, target->name);
            m->imports[k].module=target;
            m->imports[k].entry=entry;
            m->imports[k].is_native=false;
            free(fname);
        } else if(m->pool[k].tag=='X') {
            size_t lib_id=m->pool[k].operand;
            char *libname=m->pool[lib_id].name;
            ds so_path=strdup(getenv("CHKALOV"));
            ds_cat(&so_path, "/lib/", libname, "/lib", libname, ".so", NULL);
            void *h=dlopen(so_path, RTLD_LAZY);
            free(so_path);
            if(!h) error("Cannot load: %s", dlerror());
            void *func=dlsym(h, m->pool[k].name);
            if(!func) { dlclose(h); error("Undefined: %s", m->pool[k].name); }
            m->imports[k].is_native=true;
            m->imports[k].native_func=func;
        }
    }
}
