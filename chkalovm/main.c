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

int main(int argc, char **argv) {
    bool debug=false;
    uint32_t magic, len;
    size_t hs, hp=0, i=0, bp=0, j=0;
    uint8_t isize, o;
    FILE *a;
    char *heap, *c, **pool=NULL;
    int64_t v, g, h, f, *p;
    Vm *P, *end;
    cv vm, raw, stack, vars, frames, args;
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

    heap=malloc(hdr.heap_size);
    fread(heap, 1, hdr.heap_size, a);

    for(i=0; i<hdr.pool_size; i++) {
        fread(&len, 1, sz(uint32_t), a);
        pool=realloc(pool, (i+1)*sizeof(char *));
        pool[i]=malloc(len);
        fread(pool[i], 1, len, a);
        puts(pool[i]);
    }

    cv_resize(&raw, hdr.code_size);
    fread(raw.d, 1, hdr.code_size, a);

    c=raw.d;

    i=0;
    while(i<raw.s) {
        Vm q;
        q.opcode=c[i++];
        isize=c[i++];
        int64_t tmp=0;
        memcpy(&tmp, c+i, isize);
        switch(isize) {
            case 1: tmp=(int8_t)tmp; break;
            case 2: tmp=(int16_t)tmp; break;
            case 4: tmp=(int32_t)tmp; break;
            case 8: tmp=(int64_t)tmp; break;
        }
        q.value=tmp;
        cv_push(&vm, &q);
        i+=isize;
        printf("opcode = %zx, value = %zx\r\n", q.opcode, q.value);
    }

    P=((Vm *)vm.d)+hdr.entry_point;
    end=((Vm *)vm.d)+vm.s;
    cv_free(&raw);
    fclose(a);

    printf("vm.s = %zu, entry_point = %llu\n", vm.s, hdr.entry_point);

    while(P<end) {
        o=P->opcode;
        v=P->value;
        printf("stack: s=%zx c=%zu es=%zu d=%p\n", stack.s, stack.c, stack.es, (void*)stack.d);
        switch(o) {
            case PUSH:
            case S_PUSH: {
                cv_push(&stack, &v);
                if(debug) puts("push!");
                break;
            }
            case CALL: {
                if(debug) puts("call");
                ds m, l, r;
                r=heap+v;
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
                    break;
                }
                int64_t a;
                cv_pop(&stack, &f);
                int64_t args[16];
                for(size_t k=0; k<f; k++) {
                    cv_pop(&stack, &args[k]);
                }
                if(debug) printf("CALL: a=%llx\n", a);
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

           case STORE: {
                int64_t val;
                if(debug) puts("store");
                cv_pop(&stack, &val);
                if(bp+v>=vars.s) {
                    cv_resize(&vars, bp+v+1);
                }
                *(int64_t *)(cv_eptr(&vars, bp+v))=val;
                break;
            }
            case ADD: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                g+=h;
                cv_push(&stack, &g);
                break;
            }
            case SUB: {
                cv_pop(&stack, &h);
                cv_pop(&stack, &g);
                g-=h;
                cv_push(&stack, &g);
                break;
            }
            case MUL: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                g*=h;
                cv_push(&stack, &g);
                break;
            }
            case DIV: {
                cv_pop(&stack, &h);
                cv_pop(&stack, &g);
                g/=h;
                cv_push(&stack, &g);
                break;
            }
            case LOAD: {
                if(debug) puts("load");
                if(bp+v>=vars.s) error("Index out of bounds: %llx", bp+v);
                cv_push(&stack, cv_eptr(&vars, bp+v));
                break;
            }
            case CMP_EQ: {
                if(debug) puts("cmp_eq");
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                f=g==h;
                cv_push(&stack, &f);
                break;
            }
            case CMP_NEQ: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                f=g!=h;
                cv_push(&stack, &f);
                break;
            }
            case CMP_BT: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                f=h>g;
                cv_push(&stack, &f);
                break;
            }
            case CMP_LT: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                f=h<g;
                cv_push(&stack, &f);
                break;
            }
            case CMP_BE: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                f=h>=g;
                cv_push(&stack, &f);
                break;
            }
            case CMP_LE: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                f=h<=g;
                cv_push(&stack, &f);
                break;
            }
            case DUP: {
                cv_push(&stack, cv_back(&stack));
                break;
            }
            case JMP_IF: {
                if(debug) puts("jmp_if");
                cv_pop(&stack, &f);
                if(f) P=((Vm *)vm.d)+v;
                else P++;
                continue;
            }
            case JMP_IFN: {
                if(debug) puts("jmp_ifn");
                cv_pop(&stack, &f);
                if(!f) P=((Vm *)vm.d)+v;
                else P++;
                continue;
            }
            case JUMP: {
                P=((Vm *)vm.d)+v;
                continue;
            }
            case INVOKE: {
                Frame f;
                f.vars_s=vars.s;
                f.args_s=args.s;
                f.ret_addr=P+1;
                cv_push(&frames, &f);

                for(size_t k=0; k<args.s; k++) {
                    cv_push(&vars, cv_eptr(&args, k));
                }
                args.s=0;
                bp=f.vars_s;
                P=(Vm *)vm.d+v;
                continue;
            }
            case RET: {
                Frame *fr=cv_back(&frames);
                vars.s=fr->vars_s;
                args.s=fr->args_s;
                bp=fr->vars_s;
                P=fr->ret_addr;
                frames.s--;
                continue;
            }
            case ARG: {
                int64_t arg;
                cv_pop(&stack, &arg);
                cv_push(&args, &arg);
                break;
            }
            default: {
                printf("VM warning: incorrect opcode ( %x / %d )", o, o);
                break;
            }
        }
        P++;
    }
    if(debug) printf("Ended on instruction pointer %llx \r\n", i);
    if(debug) {
        printf("Heap (hp=%llx): ", hp);
        for(int i=0; i<hp; i++) {
            printf("%c", heap[i]?heap[i]:'$');
        }
        puts("\r\nHeap end");
    }

    cv_free(&vm);
    cv_free(&vars);
    cv_free(&stack);
    cv_free(&frames);
    free(heap);
    return 0;
}
