#include "../chkalov.h"
#include "../chkapi.h"

void *env_alloc(ChkEnv *e, size_t s) {
    printf("Env_alloc! \r\n");
    if(s>=*e->_hs) {
        *e->_heap=realloc(*e->_heap, *e->_hs=s<<1);
    }
    void *ret=*e->_heap+*e->_hp;
    *e->_hp+=s;
    return ret;
}

int main(int argc, char **argv) {
    bool debug=false;

    if (!getenv("CHKALOV")) {
        error("Not setted environment variable CHKALOV!\n");
        return 0;
    }

    FILE *a;
    a=fopen(argv[1], "rb");
    if(argc>=3) debug=CSUB(argv[2], "-d", 2);
    if(!a) error("Not found CVM file!");
    uint32_t magic;
    uint64_t t;
    fread(&magic, 1, sz(uint32_t), a);
    if(magic!=0x05020100) error("Hex magic number incorrect!");

    cv vm;
    Pool *p;
    cv_init(&vm, 8, sz(unsigned char));
    uint64_t i,b;
    cv pool;
    char *heap=malloc(1024);
    size_t hs=1024, hp=0;
    cv_init(&pool, 8, sz(Pool));
    cv stack, vars;
    cv_init(&stack, 8, sz(Slot));
    cv_init(&vars, 8, sz(Slot));
    fread(&t, 1, sz(uint64_t), a);
    cv_resize(&pool, t);
    for(i=0; i<t; i++) {
        p=(Pool *)cv_eptr(&pool, i);
        fread(&p->type, 1, sizeof(unsigned char), a);
        fread(&b, 1, sz(b), a);
        p->value=malloc(b+1);
        fread(p->value, 1, b, a);
        if(debug) printf("Readed: pool [%d] as value %s ! \r\n", i, p->value);
    }
    fread(&t, 1, sz(t), a);
    printf("Bytecode size: %llx \r\n", t);
    cv_resize(&vm, t);
    fread(vm.d, 1, t, a);
    int64_t tmp;
    if(debug) printf("t: %d ! \r\n", t);
    unsigned char o,ty;
    int64_t v;
    i=0;
    Slot g, h, f;
    unsigned char *c=vm.d;
    while(i<t) {
        o=c[i++];
        ty=c[i++];
        v=0;
        memcpy(&v, (unsigned char*)vm.d+i, ty&0x0f);
        i+=ty&0x0f;
        switch(o) {
            case PUSH: {
                Slot az;
                az.type=ty;
                az.value=v;
                cv_push(&stack, &az);
                if(debug) puts("push!");
                break;
            }
            case CALL: {
                //if(debug) cout << "Stack size: " << stack.size() << endl;
                /*for(int i = 0; i < stack.size(); i++) {
                    if(debug) cout << "  [" << i << "] type=" << (int)stack[i].type << " value=" << stack[i].value << endl;
                }*/
                ds m, l, r;
                Pool *p=(Pool*)cv_eptr(&pool, v);
                r=p->value;
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
                typedef Slot (*CFUNC)(size_t argc, Slot *argp, ChkEnv *env);
                CFUNC func = (CFUNC)dlsym(h, l);
                if(!func) {
                    //cerr << "dlsym failed for '" << l << "': " << dlerror() << endl;
                    dlclose(h);
                    break;
                }
                Slot a;
                cv_pop(&stack, &a);
                if(debug) printf("CALL: a.type=0x%02x, a.value=%llx\n", a.type, (long long)a.value);
                //if(debug) cout << "calling... a.value=" << a.value << " \n";
                if(ISSTR(a.type)) a.value=(int64_t)cv_eptr(&pool, a.value);
                //if(debug) cout << "calling... a.value (converted!)=" << a.value << " \n";
                ChkEnv env;
                env._pool=&pool;
                env._heap=&heap;
                env._hp=&hp;
                env._hs=&hs;
                env.alloc=env_alloc;
                a=func(1, &a, &env);
                if(a.type) cv_push(&stack, &a);
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
                Slot val;
                cv_pop(&stack, &val);
                if(v>=vars.s) {
                    cv_resize(&vars, v+1);
                }
                *(Slot *)(cv_eptr(&vars, v))=val;
                break;
            }
            case ADD: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                f.value=g.value+h.value;
                cv_push(&stack, &f);
                break;
            }
            case SUB: {
                cv_pop(&stack, &h);
                cv_pop(&stack, &g);
                f.value=g.value-h.value;
                cv_push(&stack, &f);
                break;
            }
            case MUL: {
                cv_pop(&stack, &g);
                cv_pop(&stack, &h);
                f.value=g.value*h.value;
                cv_push(&stack, &f);
                break;
            }
            case DIV: {
                cv_pop(&stack, &h);
                cv_pop(&stack, &g);
                if(!h.value) error("Division by zero");
                f.value=g.value/h.value;
                cv_push(&stack, &f);
                break;
            }
            case LOAD: {
                if(!(v<vars.s)) error("Index out of bounds: %llx", v);
                cv_push(&stack, cv_eptr(&vars, v));
                break;
            }
            case ALLOC: {
                if(debug) printf("Alloc: type %x, value %x, heap size %x, heap pointer %x \r\n", ty, v, hs, hp);
                size_t sz=v;
                if(sz>=hs) {
                    heap=realloc(heap, hs=sz<<1);
                    if(debug) printf("Reallocated: heap size %x, heap pointer %x \r\n", hs, hp);
                }
                g.value=hp;
                hp+=sz;
                cv_push(&stack, &g);
                break;
            }
            case SETFIELD: {
                cv_pop(&stack, &g); // base pointer
                cv_pop(&stack, &h); // value
                if(debug) printf("Setfield: base pointer %x, offset %x \r\n", g.value, v);
                memcpy(heap+g.value+v, &h.value, h.type&0x0f);
                break;
            }

            case GETFIELD: {
                /*size_t off=0;
                uint8_t sb;*/
                cv_pop(&stack, &g); // base pointer
                /*memcpy(&off, (uint8_t *)&v, (ty&0x0f)-1);
                printf("offset size: %x\r\n", (ty&0x0f)-1);
                memcpy(&sb, ((uint8_t *)&v)+((ty&0x0f)-1), 1);
                if(debug) printf("Getfield: base pointer %x, offset %x, common byte %x \r\n", g.value, off, sb);*/
                f.value=0;
                memcpy(&f.value, (char*)heap+g.value+v, 1);
                if(debug) printf("f.value: %llx \r\n", f.value);
                cv_push(&stack, &f);
                break;
            }
        }
    }
    cv_free(&stack);
    printf("Heap: ");
    for(int i=0; i<hp; i++) {
        printf("%c", heap[i]);
    }
    puts("\r\nHeap end");
    free(heap);
    cv_free(&vars);
    cv_free(&pool);
    return 0;
}
