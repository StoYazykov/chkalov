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

    char *opcodes[]={
    /* 0x00 */ "(incorrect)",
    /* 0x01 */ "(m_inc)",
    /* 0x02 */ "(m_dec)",
    /* 0x03 */ "(unused)",
    /* 0x04 */ "(unused)",
    /* 0x05 */ "(unused)",
    /* 0x06 */ "(unused)",
    /* 0x07 */ "load",
    /* 0x08 */ "push",
    /* 0x09 */ "push",
    /* 0x0A */ "store",
    /* 0x0B */ "inc",
    /* 0x0C */ "dec",
    /* 0x0D */ "add",
    /* 0x0E */ "sub",
    /* 0x0F */ "mul",
    /* 0x10 */ "div",
    /* 0x11 */ "call",
    /* 0x12 */ "cmp_eq",
    /* 0x13 */ "cmp_neq",
    /* 0x14 */ "cmp_lt",
    /* 0x15 */ "cmp_bt",
    /* 0x16 */ "cmp_be",
    /* 0x17 */ "cmp_le",
    /* 0x18 */ "jump",
    /* 0x19 */ "alloc",
    /* 0x1A */ "setfield",
    /* 0x1B */ "getfield",
    /* 0x1C */ "dup",
    /* 0x1D */ "jmp_if",
    /* 0x1E */ "jmp_ifn",
    /* 0x1F */ "invoke",
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
    printf("Bytecode size (in bytes): %llXh / %lld \r\n", t, t);
    cv_resize(&vm, t);
    fread(vm.d, 1, t, a);
    unsigned char *c=vm.d;
    uint8_t o, isize;
    int64_t v;
    size_t i=0;

    while(i<t) {
        o=c[i++];
        isize=c[i++];
        v=0;
        memcpy(&v, c+i, isize);
        i+=isize;

        printf("%-10s", opcodes[o]);
        printf("%10llXh / %-10lld", v, v);
        if(o==CALL||o==S_PUSH) printf("%10s %s", "//", p+v);
        printf(" \r\n");
    }
PEND:
    fclose(a);
    cv_free(&vm);
    free(p);
    return 0;
}
