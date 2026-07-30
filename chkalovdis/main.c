#include "../chkalov.h"

int main(int argc, char **argv) {
    if(argc<3) {
        puts(
        "Chkalov Bytecode Disassembler (version " VERSION ")\r\n"
        "Usage: ./chkdis <flags> <filename>\r\n"
        "Flags: \r\n"
        "h - view heap image\r\n"
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
        "cmp_eq",
        "cmp_neq",
        "cmp_lt",
        "cmp_bt",
        "cmp_be",
        "cmp_le",
        "jump",
        "setfield",
        "getfield",
        "dup",
        "jmp_if",
        "jmp_ifn"
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
    char *p;
    uint8_t vp=!!strchr(argv[1], 'h'), vb=!!strchr(argv[1], 'b');
    cv_init(&vm, 8, sz(unsigned char));
    uint64_t i,b;
    fread(&t, 1, sz(uint64_t), a);
    p=malloc(t);
    fread(p, 1, t, a);
    if(vp) {
        printf("Heap (hp=%llx): ", t);
        for(int i=0; i<t; i++) {
            printf("%c", p[i]?p[i]:'$');
        }
        puts("\r\nHeap end");
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
        printf(" %-4x", i);
        o=c[i++];
        ty=c[i++];
        v=0;
        memcpy(&v, c+i, ty&0x0f);
        i+=ty&0x0f;
        printf(":  %-10s", opcodes[o]);
        if(ty) printf("%-10s%-10llx(%u)", types[(ty&0xf0)>>4], v, ty&0x0F);
        printf(" \r\n");
    }
PEND:
    fclose(a);
    return 0;
}
