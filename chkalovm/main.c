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
    uint32_t magic;
    size_t t, hs, hp=0, i=0;
    uint8_t isize, o;
    FILE *a;
    char *heap, *c;
    int64_t v, g, h, f, *p;
    Vm *P, *end;
    cv vm, raw, stack, vars;

    if (!getenv("CHKALOV")) {
        error("Not setted environment variable CHKALOV!\n");
        return 0;
    }

    puts("Chkalov Virtual Machine (version " VERSION ")\r\n");

    a=fopen(argv[1], "rb");
    if(argc>=3) debug=CSUB(argv[2], "-d", 2);
    if(!a) error("Not found CVM file!");

    fread(&magic, 1, sz(uint32_t), a);
    if(magic!=0x05020200) error("Hex magic number incorrect!");

    cv_init(&vm, 8, sz(Vm));
    cv_init(&raw, 8, 1);
    cv_init(&stack, 8, sz(int64_t));
    cv_init(&vars, 8, sz(int64_t));

    fread(&t, 1, sz(uint64_t), a);
    heap=malloc((hs=(t<<1)));

    fread(heap, 1, t, a);
    hp+=t;

    fread(&t, 1, sz(t), a);
    if(debug) printf("Bytecode size: %llx \r\n", t);
    cv_resize(&raw, t);
    fread(raw.d, 1, t, a);
    c=raw.d;

    while(i<raw.s) {
        Vm q;
        q.opcode=c[i++];
        isize=c[i++];
        q.value=0;
        memcpy(&q.value, c+i, isize);
        cv_push(&vm, &q);
        i+=isize;
    }

    P=(Vm *)vm.d;
    end=((Vm *)vm.d)+vm.s;

    while(P<end) {
        o=P->opcode;
        v=P->value;
        switch(o) {
            case PUSH: {
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
                cv_pop(&stack, &a);
                if(debug) printf("CALL: a=%llx\n", a);
                ChkEnv env;
                env._heap=&heap;
                env._hp=&hp;
                env._hs=&hs;
                env.alloc=env_alloc;
                g=func(1, &a, &env);
                cv_push(&stack, &g);
                dlclose(h);
                break;
            }
            // case IFNE: {
            //     int64_t a=pop(stack).value, b=pop(stack).value;
            //     cout << "IFNE; " << a << " and " << b << '!' << endl;
            //     if(a!=b) {
            //         cout << hex << "!=. Jumping to " << v << "..." << '!' << dec << endl;
            //         i=v;
            //     }
            //     break;
            // }
           case STORE: {
                int64_t val;
                if(debug) puts("store");
                cv_pop(&stack, &val);
                if(v>=vars.s) {
                    cv_resize(&vars, v+1);
                }
                *(int64_t *)(cv_eptr(&vars, v))=val;
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
                if(!(v<vars.s)) error("Index out of bounds: %llx", v);
                cv_push(&stack, cv_eptr(&vars, v));
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
                f=g>h;
                cv_push(&stack, &f);
                break;
            }
            case CMP_LT: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                f=g<h;
                cv_push(&stack, &f);
                break;
            }
            case CMP_BE: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                f=g>=h;
                cv_push(&stack, &f);
                break;
            }
            case CMP_LE: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                f=g<=h;
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
        }
        P++;
    }
    if(debug) printf("Ended on instruction pointer %llx \r\n", i);
    cv_free(&stack);
    if(debug) {
        printf("Heap (hp=%llx): ", hp);
        for(int i=0; i<hp; i++) {
            printf("%c", heap[i]?heap[i]:'$');
        }
        puts("\r\nHeap end");
    }
    free(heap);
    cv_free(&vars);
    return 0;
}
