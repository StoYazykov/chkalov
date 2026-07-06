#include "../chkalov.h"

int main(int argc, char **argv) {
    bool debug;

    if (!getenv("CHKALOV")) {
        error("Not setted environment variable CHKALOV!\n");
        return 0;
    }

    debug=true;
    FILE *a;
    a=fopen(argv[1], "rb");
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
                puts("push!");
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
                typedef Slot (*CFUNC)(size_t argc, Slot *argp);
                CFUNC func = (CFUNC)dlsym(h, "println");
                if(!func) {
                    //cerr << "dlsym failed for '" << l << "': " << dlerror() << endl;
                    dlclose(h);
                    break;
                }
                Slot a;
                cv_pop(&stack, &a);
                printf("CALL: a.type=0x%02x, a.value=%lld\n", a.type, (long long)a.value);
                //if(debug) cout << "calling... a.value=" << a.value << " \n";
                Pool *pa=cv_eptr(&pool, a.value);
                if(ISSTR(a.type)) a.value=(int64_t)pa->value;
                //if(debug) cout << "calling... a.value (converted!)=" << a.value << " \n";
                func(1, &a);
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
            case LOAD: {
                cv_push(&stack, cv_eptr(&vars, v));
                break;
            }
        }
    }
    return 0;
}
