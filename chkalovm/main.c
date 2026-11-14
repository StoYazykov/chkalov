/*
 * Copyright (C) 2025-2026  StoYazykov
 *
 * This file is part of Chkalov.
 *
 * Chkalov is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Chkalov is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Chkalov.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "../chkalov.h"
#include "../chkapi.h"

typedef struct {
    size_t vars_s, args_s;
    Vm *ret_addr;
} Frame;

void *env_alloc(ChkEnv *e, size_t s) {
    //printf("Env_alloc! \r\n");
    if(s>=*e->_hs) {
        *e->_heap=realloc(*e->_heap, *e->_hs=s<<1);
    }
    void *ret=*e->_heap+*e->_hp;
    *e->_hp+=s;
    return ret;
}

cv vm, raw, stack, vars, frames, args;

void push(int64_t p) {
    cv_push(&stack, &p);
}

int main(int argc, char **argv) {
    int8_t temp_c;
    int16_t temp_s;
    int32_t temp_i;
    int64_t temp_l;

    bool debug=false;
    uint32_t magic, len;
    size_t hs, hp=0, i=0, bp=0, j=0;
    uint8_t isize, o;
    FILE *a;
    char *heap, *c, **raw_pool;
    int64_t v, g, h, f, *p;
    Vm *P, *end;
    Header hdr;

    if (!getenv("CHKALOV")) {
        error("Not setted environment variable CHKALOV!\n");
        return 0;
    }

    puts("Chkalov Virtual Machine (version " VERSION ")\r\n");

    a=fopen(argv[1], "rb");
    if(argc>=3) debug=!!CSUB(argv[2], "-d", 2);
    if(!a) error("Not found CVM file!");

    fread(&hdr, 1, sz(Header), a);
    if(hdr.magic!=0x05020200) error("Hex magic number incorrect!");

    cv_init(&vm, 8, sz(Vm));
    cv_init(&raw, 8, 1);
    cv_init(&stack, 8, sz(int64_t));
    cv_init(&vars, 8, sz(int64_t));
    cv_init(&frames, 4, sz(Frame));
    cv_init(&args, 4, sz(int64_t));

    hp=hdr.heap_size;
    heap=malloc(hdr.heap_size);
    fread(heap, 1, hdr.heap_size, a);

    raw_pool=malloc(hdr.pool_size*sz(char *));

    for(i=0; i<hdr.pool_size; i++) {
        fread(&len, 1, sz(uint32_t), a);
        raw_pool[i]=malloc(len);
        fread(raw_pool[i], 1, len, a);
    }

    cv_resize(&raw, hdr.code_size);
    fread(raw.d, 1, hdr.code_size, a);

    c=raw.d;

    i=0;


    int operand_sizes_base[]={
        1, // CPUSH
        2, // SPUSH
        4, // IPUSH
        8, // LPUSH
        8, // APUSH
        0, // POP
        0, // DUP
        0, // SWAP
        8, // JUMP
        8, // JMP_IF
        8, // JMP_IFN
        8, // CALL
        0, // RET
        0, // ARG
        8, // INVOKE
        8, // INVOKE_EXT
        8, // ALLOC
        8  // FREE
    };

    while(i<raw.s) {
        Vm q={0};
        size_t op;
        int size;
        op=c[i++];
        q.opcode=op;
        if (op>=OP_CLOAD&&op<=OP_ASTORE) {
            size=4;
        } else if (op>=OP_CADD&&op<=OP_ANEG) {
            size=0;
        } else if (op==OP_CAST) {
            size=1;
        } else {
            size=operand_sizes_base[op];
        }
        memcpy(&q.value, (char *)raw.d+i, size);
        cv_push(&vm, &q);
        i+=size;
    }

    P=((Vm *)vm.d)+hdr.entry_point;
    end=((Vm *)vm.d)+vm.s;
    cv_free(&raw);
    fclose(a);

    while(P<end) {
        o=P->opcode;
        v=P->value;
        switch(o) {
            case OP_CPUSH: {
                push((int64_t)(int8_t)v);
                break;
            }
            case OP_SPUSH: {
                push((int64_t)(int16_t)v);
                break;
            }
            case OP_IPUSH: {
                push((int64_t)(int32_t)v);
                break;
            }
            case OP_LPUSH: {
                push((int64_t)v);
                break;
            }
            case OP_APUSH: {
                push((int64_t)(uint64_t)v);
                break;
            }
            case OP_POP: {
                cv_popr(&stack);
                break;
            }
            case OP_DUP: {
                push(*(int64_t *)cv_back(&stack));
                break;
            }
            case OP_SWAP: {
                int64_t a, b;
                cv_pop(&stack, &a);
                cv_pop(&stack, &b);
                push(a);
                push(b);
                break;
            }
            case OP_JUMP: {
                P=((Vm *)vm.d)+(uint32_t)v;
                continue;
            }
            case OP_JMP_IF: {
                cv_pop(&stack, &f);
                if(f) P=((Vm *)vm.d)+(uint32_t)v;
                else P++;
                continue;
            }
            case OP_JMP_IFN: {
                cv_pop(&stack, &f);
                if(!f) P=((Vm *)vm.d)+(uint32_t)v;
                else P++;
                continue;
            }
            case OP_CALL: {
               // printf("VM: DEBUG OP_CALL");
                ds m, l, r;
                r=heap+(uint32_t)v;
                ds d=strchr(r, '.');
                size_t ml=d-r;
                m=malloc(ml+1);
                strncpy(m, r, ml);
                m[ml]=0;
                l=strdup(d+1);
                ds s=strdup(getenv("CHKALOV"));
                ds_cat(&s, "/", m, NULL);
#ifdef _WIN32
                ds_cat(&s, m, NULL);
#else
                ds_cat(&s, "/lib", m, NULL);
#endif
#ifdef _WIN32
                ds_cat(&s, ".dll", NULL);
#else
                ds_cat(&s, ".so", NULL);
#endif
                void *h=dlopen(s, RTLD_LAZY);
                typedef Int (*CFUNC)(size_t argc, int64_t *argp, ChkEnv *env);
                CFUNC func=(CFUNC)dlsym(h, l);
                if(!func) {
                    dlclose(h);
                    error("VM: undefined function");
                }
                cv_pop(&stack, &f);
                int64_t args[16];
                for(size_t k=0; k<f; k++) {
                    cv_pop(&stack, &args[k]);
                }
                ChkEnv env;
                env._heap=&heap;
                env._hp=&hp;
                env._hs=&hs;
                env.alloc=env_alloc;
                g=func(f, args, &env);
                cv_push(&stack, &g);
                dlclose(h);
                break;
            }
            case OP_RET: {
                Frame *fr;
                if(!frames.s) {
                    P=end;
                    continue;
                }
                fr=cv_back(&frames);
                vars.s=fr->vars_s;
                args.s=fr->args_s;
                bp=fr->vars_s;
                P=fr->ret_addr;
                frames.s--;
                continue;
            }
            case OP_ARG: {
                int64_t arg;
                cv_pop(&stack, &arg);
                cv_push(&args, &arg);
                break;
            }
            case OP_INVOKE: {
                Frame fr;
                fr.vars_s=vars.s;
                fr.args_s=args.s;
                fr.ret_addr=P+1;
                cv_push(&frames, &fr);
                for(size_t k=0; k<args.s; k++) {
                    cv_push(&vars, cv_eptr(&args, k));
                }
                args.s=0;
                bp=fr.vars_s;
                P=((Vm *)vm.d)+(uint32_t)v;
                continue;
            }
            case OP_INVOKE_EXT: {
                Frame fr;
                fr.vars_s=vars.s;
                fr.args_s=args.s;
                fr.ret_addr=P+1;
                cv_push(&frames, &fr);
                for(size_t k=0; k<args.s; k++) {
                    cv_push(&vars, cv_eptr(&args, k));
                }
                args.s=0;
                bp=fr.vars_s;
                P=((Vm *)vm.d)+(uint32_t)v;
                continue;
            }
            case OP_ALLOC: {
                size_t sz_= (uint32_t)v;
                if(hp+sz_>hs) {
                    heap=realloc(heap, hs=(hp+sz_)<<1);
                }
                push((int64_t)hp);
                hp+=sz_;
                break;
            }
            case OP_FREE: {
                break;
            }
            case OP_CLOAD: {
                size_t idx=(uint32_t)v;
                if(bp+idx>=vars.s) error("CLOAD: index out of bounds: %llx", bp+idx);
                int8_t val=*(int8_t *)cv_eptr(&vars, bp+idx);
                push((int64_t)val);
                break;
            }
            case OP_SLOAD: {
                size_t idx=(uint32_t)v;
                if(bp+idx>=vars.s) error("SLOAD: index out of bounds: %llx", bp+idx);
                int16_t val=*(int16_t *)cv_eptr(&vars, bp+idx);
                push((int64_t)val);
                break;
            }
            case OP_ILOAD: {
                size_t idx=(uint32_t)v;
                if(bp+idx>=vars.s) error("ILOAD: index out of bounds: %llx", bp+idx);
                int32_t val=*(int32_t *)cv_eptr(&vars, bp+idx);
                push((int64_t)val);
                break;
            }
            case OP_LLOAD: {
                size_t idx=(uint32_t)v;
                if(bp+idx>=vars.s) error("LLOAD: index out of bounds: %llx", bp+idx);
                int64_t val=*(int64_t *)cv_eptr(&vars, bp+idx);
                push(val);
                break;
            }
            case OP_ALOAD: {
                size_t idx=(uint32_t)v;
                if(bp+idx>=vars.s) error("ALOAD: index out of bounds: %llx", bp+idx);
                uint64_t val=*(uint64_t *)cv_eptr(&vars, bp+idx);
                push((int64_t)val);
                break;
            }
            case OP_CSTORE: {
                size_t idx=(uint32_t)v;
                if(bp+idx>=vars.s) cv_resize(&vars, bp+idx+1);
                cv_pop(&stack, &g);
                *(int8_t *)cv_eptr(&vars, bp+idx)=(int8_t)g;
                break;
            }
            case OP_SSTORE: {
                size_t idx=(uint32_t)v;
                if(bp+idx>=vars.s) cv_resize(&vars, bp+idx+1);
                cv_pop(&stack, &g);
                *(int16_t *)cv_eptr(&vars, bp+idx)=(int16_t)g;
                break;
            }
            case OP_ISTORE: {
                size_t idx=(uint32_t)v;
                if(bp+idx>=vars.s) cv_resize(&vars, bp+idx+1);
                cv_pop(&stack, &g);
                *(int32_t *)cv_eptr(&vars, bp+idx)=(int32_t)g;
                break;
            }
            case OP_LSTORE: {
                size_t idx=(uint32_t)v;
                if(bp+idx>=vars.s) cv_resize(&vars, bp+idx+1);
                cv_pop(&stack, &g);
                *(int64_t *)cv_eptr(&vars, bp+idx)=g;
                break;
            }
            case OP_ASTORE: {
                size_t idx=(uint32_t)v;
                if(bp+idx>=vars.s) cv_resize(&vars, bp+idx+1);
                cv_pop(&stack, &g);
                *(uint64_t *)cv_eptr(&vars, bp+idx)=(uint64_t)g;
                break;
            }
            case OP_CADD: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(int8_t)(g+h));
                break;
            }
            case OP_CSUB: {
                cv_pop(&stack, &h);
                cv_pop(&stack, &g);
                push((int64_t)(int8_t)(g-h));
                break;
            }
            case OP_CMUL: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(int8_t)(g*h));
                break;
            }
            case OP_CDIV: {
                cv_pop(&stack, &h);
                cv_pop(&stack, &g);
                if(h==0) error("CDIV: division by zero");
                push((int64_t)(int8_t)(g/h));
                break;
            }
            case OP_CMOD: {
                cv_pop(&stack, &h);
                cv_pop(&stack, &g);
                if(h==0) error("CMOD: division by zero");
                push((int64_t)(int8_t)(g%h));
                break;
            }
            case OP_SADD: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(int16_t)(g+h));
                break;
            }
            case OP_SSUB: {
                cv_pop(&stack, &h);
                cv_pop(&stack, &g);
                push((int64_t)(int16_t)(g-h));
                break;
            }
            case OP_SMUL: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(int16_t)(g*h));
                break;
            }
            case OP_SDIV: {
                cv_pop(&stack, &h);
                cv_pop(&stack, &g);
                if(h==0) error("SDIV: division by zero");
                push((int64_t)(int16_t)(g/h));
                break;
            }
            case OP_SMOD: {
                cv_pop(&stack, &h);
                cv_pop(&stack, &g);
                if(h==0) error("SMOD: division by zero");
                push((int64_t)(int16_t)(g%h));
                break;
            }
            case OP_IADD: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(int32_t)(g+h));
                break;
            }
            case OP_ISUB: {
                cv_pop(&stack, &h);
                cv_pop(&stack, &g);
                push((int64_t)(int32_t)(g-h));
                break;
            }
            case OP_IMUL: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(int32_t)(g*h));
                break;
            }
            case OP_IDIV: {
                cv_pop(&stack, &h);
                cv_pop(&stack, &g);
                if(h==0) error("IDIV: division by zero");
                push((int64_t)(int32_t)(g/h));
                break;
            }
            case OP_IMOD: {
                cv_pop(&stack, &h);
                cv_pop(&stack, &g);
                if(h==0) error("IMOD: division by zero");
                push((int64_t)(int32_t)(g%h));
                break;
            }
            case OP_LADD: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push(g+h);
                break;
            }
            case OP_LSUB: {
                cv_pop(&stack, &h);
                cv_pop(&stack, &g);
                push(g-h);
                break;
            }
            case OP_LMUL: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push(g*h);
                break;
            }
            case OP_LDIV: {
                cv_pop(&stack, &h);
                cv_pop(&stack, &g);
                if(h==0) error("LDIV: division by zero");
                push(g/h);
                break;
            }
            case OP_LMOD: {
                cv_pop(&stack, &h);
                cv_pop(&stack, &g);
                if(h==0) error("LMOD: division by zero");
                push(g%h);
                break;
            }
            case OP_AADD: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((uint64_t)g+(uint64_t)h));
                break;
            }
            case OP_ASUB: {
                cv_pop(&stack, &h);
                cv_pop(&stack, &g);
                push((int64_t)((uint64_t)g-(uint64_t)h));
                break;
            }
            case OP_CEQ: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((int8_t)g==(int8_t)h));
                break;
            }
            case OP_CNEQ: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((int8_t)g!=(int8_t)h));
                break;
            }
            case OP_CLT: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((int8_t)h<(int8_t)g));
                break;
            }
            case OP_CBT: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((int8_t)h>(int8_t)g));
                break;
            }
            case OP_CLE: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((int8_t)h<=(int8_t)g));
                break;
            }
            case OP_CBE: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((int8_t)h>=(int8_t)g));
                break;
            }
            case OP_SEQ: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((int16_t)g==(int16_t)h));
                break;
            }
            case OP_SNEQ: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((int16_t)g!=(int16_t)h));
                break;
            }
            case OP_SLT: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((int16_t)h<(int16_t)g));
                break;
            }
            case OP_SBT: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((int16_t)h>(int16_t)g));
                break;
            }
            case OP_SLE: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((int16_t)h<=(int16_t)g));
                break;
            }
            case OP_SBE: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((int16_t)h>=(int16_t)g));
                break;
            }
            case OP_IEQ: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((int32_t)g==(int32_t)h));
                break;
            }
            case OP_INEQ: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((int32_t)g!=(int32_t)h));
                break;
            }
            case OP_ILT: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((int32_t)h<(int32_t)g));
                break;
            }
            case OP_IBT: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((int32_t)h>(int32_t)g));
                break;
            }
            case OP_ILE: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((int32_t)h<=(int32_t)g));
                break;
            }
            case OP_IBE: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((int32_t)h>=(int32_t)g));
                break;
            }
            case OP_LEQ: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(h==g));
                break;
            }
            case OP_LNEQ: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(h!=g));
                break;
            }
            case OP_LLT: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(h<g));
                break;
            }
            case OP_LBT: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(h>g));
                break;
            }
            case OP_LLE: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(h<=g));
                break;
            }
            case OP_LBE: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(h>=g));
                break;
            }
            case OP_AEQ: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((uint64_t)h==(uint64_t)g));
                break;
            }
            case OP_ANEQ: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((uint64_t)h!=(uint64_t)g));
                break;
            }
            case OP_ALT: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((uint64_t)h<(uint64_t)g));
                break;
            }
            case OP_ABT: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((uint64_t)h>(uint64_t)g));
                break;
            }
            case OP_ALE: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((uint64_t)h<=(uint64_t)g));
                break;
            }
            case OP_ABE: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((uint64_t)h>=(uint64_t)g));
                break;
            }
            case OP_CAND: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(int8_t)((int8_t)g&(int8_t)h));
                break;
            }
            case OP_COR: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(int8_t)((int8_t)g|(int8_t)h));
                break;
            }
            case OP_CXOR: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(int8_t)((int8_t)g^(int8_t)h));
                break;
            }
            case OP_CNOT: {
                cv_pop(&stack, &g);
                push((int64_t)(int8_t)(~(int8_t)g));
                break;
            }
            case OP_SAND: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(int16_t)((int16_t)g&(int16_t)h));
                break;
            }
            case OP_SOR: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(int16_t)((int16_t)g|(int16_t)h));
                break;
            }
            case OP_SXOR: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(int16_t)((int16_t)g^(int16_t)h));
                break;
            }
            case OP_SNOT: {
                cv_pop(&stack, &g);
                push((int64_t)(int16_t)(~(int16_t)g));
                break;
            }
            case OP_IAND: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(int32_t)((int32_t)g&(int32_t)h));
                break;
            }
            case OP_IOR: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(int32_t)((int32_t)g|(int32_t)h));
                break;
            }
            case OP_IXOR: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(int32_t)((int32_t)g^(int32_t)h));
                break;
            }
            case OP_INOT: {
                cv_pop(&stack, &g);
                push((int64_t)(int32_t)(~(int32_t)g));
                break;
            }
            case OP_LAND: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push(g&h);
                break;
            }
            case OP_LOR: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push(g|h);
                break;
            }
            case OP_LXOR: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push(g^h);
                break;
            }
            case OP_LNOT: {
                cv_pop(&stack, &g);
                push(~g);
                break;
            }
            case OP_AAND: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((uint64_t)g&(uint64_t)h));
                break;
            }
            case OP_AOR: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((uint64_t)g|(uint64_t)h));
                break;
            }
            case OP_AXOR: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((uint64_t)g^(uint64_t)h));
                break;
            }
            case OP_ANOT: {
                cv_pop(&stack, &g);
                push((int64_t)(~(uint64_t)g));
                break;
            }
            case OP_CSHL: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(int8_t)((int8_t)g<<(h&7)));
                break;
            }
            case OP_CSHR: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(int8_t)((int8_t)g>>(h&7)));
                break;
            }
            case OP_SSHL: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(int16_t)((int16_t)g<<(h&15)));
                break;
            }
            case OP_SSHR: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(int16_t)((int16_t)g>>(h&15)));
                break;
            }
            case OP_ISHL: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(int32_t)((int32_t)g<<(h&31)));
                break;
            }
            case OP_ISHR: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)(int32_t)((int32_t)g>>(h&31)));
                break;
            }
            case OP_LSHL: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push(g<<(h&63));
                break;
            }
            case OP_LSHR: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push(g>>(h&63));
                break;
            }
            case OP_ASHL: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((uint64_t)g<<(h&63)));
                break;
            }
            case OP_ASHR: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                push((int64_t)((uint64_t)g>>(h&63)));
                break;
            }
            case OP_CNEG: {
                cv_pop(&stack, &g);
                push((int64_t)(int8_t)(-(int8_t)g));
                break;
            }
            case OP_SNEG: {
                cv_pop(&stack, &g);
                push((int64_t)(int16_t)(-(int16_t)g));
                break;
            }
            case OP_INEG: {
                cv_pop(&stack, &g);
                push((int64_t)(int32_t)(-(int32_t)g));
                break;
            }
            case OP_LNEG: {
                cv_pop(&stack, &g);
                push(-g);
                break;
            }
            case OP_ANEG: {
                cv_pop(&stack, &g);
                push((int64_t)(-(uint64_t)g));
                break;
            }
            case OP_CAST: {
                uint8_t types=(uint8_t)v;
                uint8_t from=types&0x0F;
                uint8_t to=(types>>4)&0x0F;
                cv_pop(&stack, &g);
                switch(from) {
                    case 0: g=(int8_t)g; break;
                    case 1: g=(int16_t)g; break;
                    case 2: g=(int32_t)g; break;
                    case 3: g=(int64_t)g; break;
                    case 4: g=(uint64_t)g; break;
                }
                switch(to) {
                    case 0: g=(int64_t)(int8_t)g; break;
                    case 1: g=(int64_t)(int16_t)g; break;
                    case 2: g=(int64_t)(int32_t)g; break;
                    case 3: g=(int64_t)g; break;
                    case 4: g=(int64_t)(uint64_t)g; break;
                }
                push(g);
                break;
            }
            default: {
                printf("VM error: incorrect opcode ( %x )\r\n", o, o);
                printf("ip=%zx stack: s=%zx c=%zx es=%zx d=%p\n", P-(Vm *)vm.d, stack.s, stack.c, stack.es, (void*)stack.d);
                printf("stack: ");
                for(i=0; i<stack.s; i++) {
                    printf("%lx ", *((int64_t *)cv_eptr(&stack, i)));
                }
                puts("<< Stack Pointer\r\n(stack dump end)");
                abort();
            }
        }
        P++;
    }
    if(debug) printf("Ended on instruction pointer %llx \r\n", i);
    if(debug) {
        printf("Heap (hp=%llx): ", hp);
        for(i=0; i<hp; i++) {
            printf("%c", heap[i]?heap[i]:'$');
        }
        puts("\r\nHeap end");
    }

    cv_free(&vm);
    cv_free(&vars);
    cv_free(&stack);
    cv_free(&frames);
    //for(i=0; i<hdr.pool_size; i++) free(pool[i]);
   // free(pool);
    free(heap);
    return 0;
}
