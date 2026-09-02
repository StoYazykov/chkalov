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

    FILE *a=fopen(argv[2], "rb");
    uint32_t magic;
    uint64_t t;
    fread(&magic, 1, sz(uint32_t), a);
    if(magic!=0x05020200) error("Hex magic number incorrect!");
    cv vm;
    char *p;
    uint8_t vp=!!strchr(argv[1], 'h'), vb=!!strchr(argv[1], 'b');
    cv_init(&vm, 8, sz(unsigned char));
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
    size_t size=t;
    printf("Bytecode size (in bytes): %llx \r\n", t);
    cv_resize(&vm, t);
    fread(vm.d, 1, t, a);
    unsigned char *c = vm.d;
    uint8_t o, isize;
    int64_t v;
    size_t i=0;

    while(i<t) {
        o=c[i++];
        isize=c[i++];
        v=0;
        memcpy(&v, c+i, isize);
        i+=isize;

        printf(":  %-10s", opcodes[o]);
        printf("%-10x%-10llx", isize, v);
        printf(" \r\n");
    }
PEND:
    fclose(a);
    return 0;
}
