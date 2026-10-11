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
    /* 0x20 */ "ret",
    /* 0x21 */ "arg"
    };

    FILE *a=fopen(argv[2], "rb");
    if(!a) error("Not found CVM file!");
    Header hdr;
    char **pool=NULL;
    uint32_t len;
    size_t i;
    fread(&hdr, 1, sz(Header), a);
    if(hdr.magic!=0x05020200) error("Hex magic number incorrect!");

    uint8_t vp=!!strchr(argv[1], 'h'), vb=!!strchr(argv[1], 'b');

    char *heap=malloc(hdr.heap_size);
    fread(heap, 1, hdr.heap_size, a);

    for(i=0; i<hdr.pool_size; i++) {
        fread(&len, 1, sz(uint32_t), a);
        pool=realloc(pool, (i+1)*sizeof(char *));
        pool[i]=malloc(len);
        fread(pool[i], 1, len, a);
        puts(pool[i]);
    }

    if(vp) {
        printf("Heap (hp=%llx): ", hdr.heap_size);
        for(size_t i=0; i<hdr.heap_size; i++) {
            printf("%c", heap[i]?heap[i]:'$');
        }
        puts("\r\nHeap end");
    }

    if(!vb) goto PEND;

    cv vm, raw;
    cv_init(&vm, 8, sz(Vm));
    cv_init(&raw, 8, 1);

    printf("Entry point: %llXh / %lld \r\n", hdr.entry_point, hdr.entry_point);
    printf("Bytecode size (in bytes): %llXh / %lld \r\n", hdr.code_size, hdr.code_size);

    cv_resize(&raw, hdr.code_size);
    fread(raw.d, 1, hdr.code_size, a);
    unsigned char *c=raw.d;

    i=0;
    while(i<raw.s) {
        Vm q;
        q.opcode=c[i++];
        uint8_t isize=c[i++];
        q.value=0;
        memcpy(&q.value, c+i, isize);
        i+=isize;
        cv_push(&vm, &q);
    }

    Vm *P=(Vm *)vm.d;
    Vm *end=((Vm *)vm.d)+vm.s;

    while(P<end) {
        uint8_t o=P->opcode;
        int64_t v=P->value;

        printf("%-10llu %-10s", P-((Vm *)vm.d), opcodes[o]);
        printf("%10llXh / %-10lld", v, v);
        if(o==CALL||o==S_PUSH) printf("// %s", heap+v);
        if(hdr.entry_point==(P-((Vm *)vm.d))) printf("<- Entry Point");
        printf(" \r\n");

        P++;
    }

    cv_free(&raw);
    cv_free(&vm);

PEND:
    fclose(a);
    free(heap);
    return 0;
}
