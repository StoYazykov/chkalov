#include "../chkalov.h"

int main(int argc, char **argv) {
    if(argc<3) {
        puts(
        "Chkalov Bytecode Disassembler version 1.00.\r\n"
        "Usage: ./chkdis <flags> <filename>\r\n"
        "Flags: \r\n"
        "p - view pool\r\n"
        "b - view bytecode\r\n"
        );
        return 0;
    }

    uint64_t opcodes[]={
        0, 0, 0, 0, 0, 0, 0, 0,
        "load",
        "push",
        "store",
        "inc",
        "dec",
        "add",
        "sub",
        "mul",
        "div",
        "call",
        "alloc",
        "ife",
        "ifne",
        "ifl",
        "ifb",
        "ifbe",
        "ifle",
        "setfield",
        "getfield"
    };
    char *types[]={
        0,
        "char",
        "xshort",
        "short",
        "int",
        "long",
        "string",
        "library",
        "pointer",
        "index",
        "nol"
    };
    FILE *a=fopen(argv[2], "rb");
    uint32_t magic;
    uint64_t t;
    fread(&magic, 1, sz(uint32_t), a);
    if(magic!=0x05020100) error("Hex magic number incorrect!");
    cv vm;
    Pool *p;
    uint8_t vp=!!strchr(argv[1], 'p'), vb=!!strchr(argv[1], 'b');
    cv_init(&vm, 8, sz(unsigned char));
    uint64_t i,b;
    cv pool;
    cv_init(&pool, 8, sz(Pool));
    fread(&t, 1, sz(uint64_t), a);
    cv_resize(&pool, t);
    if(vp) printf("  Pool size: %x \r\n", t);
    for(i=0; i<t; i++) {
        p=(Pool *)cv_eptr(&pool, i);
        fread(&p->type, 1, sizeof(unsigned char), a);
        fread(&b, 1, sz(b), a);
        p->value=malloc(b+1);
        fread(p->value, 1, b, a);
        if(vp) printf(" #%x %s \r\n", i, p->value);
    }
    if(!vb) goto PEND;
    fread(&t, 1, sz(t), a);
    printf("Bytecode size: %llx \r\n", t);
    cv_resize(&vm, t);
    fread(vm.d, 1, t, a);
    unsigned char *c=vm.d;
    uint8_t o, ty;
    int64_t v;
    i=0;
    while(i<t) {
        o=c[i++];
        ty=c[i++];
        v=0;
        memcpy(&v, c+i, ty&0x0f);
        i+=ty&0x0f;
        printf(" %-4x:  %-10s", i, opcodes[o]);
        if(ty) printf("%-10s%llx", types[(ty&0xf0)>>4], v);
        printf(" \r\n");
    }
PEND:
    fclose(a);
    return 0;
}
