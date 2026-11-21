#include "../chkalov.h"
#include "../chkapi.h"
#include "vm_modules.h"

static cv stack, vars, frames, args;
static VMState vm_state;
static Module *current_module;

void *env_alloc(ChkEnv *e, size_t s) {
    if(s>=*e->_hs) {
        *e->_heap=realloc(*e->_heap, *e->_hs=s<<1);
    }
    void *ret=*e->_heap+*e->_hp;
    *e->_hp+=s;
    return ret;
}

void push(int64_t p) {
    cv_push(&stack, &p);
}

int main(int argc, char **argv) {
    bool debug=false;
    size_t bp=0, i=0;
    int64_t v, g, h, f;
    Vm *P, *end;
    uint8_t o;

    if(!getenv("CHKALOV")) {
        error("Not setted environment variable CHKALOV!\n");
        return 0;
    }

    puts("Chkalov Virtual Machine (version " VERSION ")\r\n");

    if(argc<2) error("Usage: chkvm <file.cvm> [-d]");
    if(argc>=3) debug=!!CSUB(argv[2], "-d", 2);

    cv_init(&stack, 8, sz(int64_t));
    cv_init(&vars, 8, sz(int64_t));
    cv_init(&frames, 4, sz(Frame));
    cv_init(&args, 4, sz(int64_t));

    vm_state.heap=malloc(16);
    vm_state.heap_size=16;
    vm_state.heap_ptr=0;

    vm_state.main_module=load_module_path(&vm_state, argv[1], argv[1]);
    resolve_main(&vm_state, vm_state.main_module);
    vm_state.current_module=vm_state.main_module;
    current_module=vm_state.current_module;

    P=current_module->code+current_module->entry_point;
    end=current_module->code+current_module->code_size;

    puts("VM Main Loop starting...");

    while(P<end) {
        o=P->opcode;
        v=P->value;
        switch(o) {
            case OP_CPUSH: push((int64_t)(int8_t)v); break;
            case OP_SPUSH: push((int64_t)(int16_t)v); break;
            case OP_IPUSH: push((int64_t)(int32_t)v); break;
            case OP_LPUSH: push((int64_t)v); break;
            case OP_APUSH: push((int64_t)(uint64_t)v); break;
            case OP_POP: cv_popr(&stack); break;
            case OP_DUP: push(*(int64_t *)cv_back(&stack)); break;
            case OP_SWAP: {
                int64_t a, b;
                cv_pop(&stack, &a);
                cv_pop(&stack, &b);
                push(a);
                push(b);
                break;
            }
            case OP_JUMP: P=current_module->code+(uint32_t)v; continue;
            case OP_JMP_IF: {
                cv_pop(&stack, &f);
                if(f) P=current_module->code+(uint32_t)v;
                else P++;
                continue;
            }
            case OP_JMP_IFN: {
                cv_pop(&stack, &f);
                if(!f) P=current_module->code+(uint32_t)v;
                else P++;
                continue;
            }
            case OP_CALL: {
                size_t X_id=(uint32_t)v;
                Module *cur=current_module;
                if(!cur->imports[X_id].is_native) error("CALL on cvm function");
                CFUNC func=(CFUNC)cur->imports[X_id].native_func;

                cv_pop(&stack, &f);
                int64_t args_buf[16];
                for(size_t k=f; k-- > 0; )
                    cv_pop(&stack, &args_buf[k]);

                ChkEnv env={0};
                env.alloc=env_alloc;
                env._heap=&vm_state.heap;
                env._hs=&vm_state.heap_size;
                env._hp=&vm_state.heap_ptr;
                g=func(f, args_buf, &env);
                cv_push(&stack, &g);
                break;
            }
            case OP_RET: {
                Frame *fr;
                if(!frames.s) { P=end; continue; }
                fr=cv_back(&frames);
                vars.s=fr->vars_s;
                args.s=fr->args_s;
                bp=fr->vars_s;
                P=fr->ret_addr;
                current_module=fr->module;
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
                Frame fr={vars.s, args.s, P+1, current_module};
                cv_push(&frames, &fr);
                for(size_t k=0; k<args.s; k++)
                    cv_push(&vars, cv_eptr(&args, k));
                args.s=0;
                bp=fr.vars_s;
                P=current_module->code+(uint32_t)v;
                continue;
            }
            case OP_INVOKE_EXT: {
                size_t F_id=(uint32_t)v;
                Module *cur=current_module;
                if(cur->imports[F_id].is_native) error("INVOKE_EXT on native");
                Module *target=cur->imports[F_id].module;
                Vm *entry=cur->imports[F_id].entry;

                Frame fr={vars.s, args.s, P+1, cur};
                cv_push(&frames, &fr);
                for(size_t k=0; k<args.s; k++)
                    cv_push(&vars, cv_eptr(&args, k));
                args.s=0;
                bp=fr.vars_s;
                current_module=target;
                P=entry;
                continue;
            }
            case OP_ALLOC: {
                size_t sz_=(uint32_t)v;
                if(vm_state.heap_ptr+sz_>vm_state.heap_size) {
                    size_t new_hs=vm_state.heap_size*2;
                    while(new_hs<vm_state.heap_ptr+sz_) new_hs*=2;
                    vm_state.heap=realloc(vm_state.heap, new_hs);
                    vm_state.heap_size=new_hs;
                }
                push((int64_t)vm_state.heap_ptr);
                vm_state.heap_ptr+=sz_;
                break;
            }
            case OP_FREE: break;
            case OP_CLOAD: {
                size_t idx=(uint32_t)v;
                int8_t val=*(int8_t *)cv_eptr(&vars, bp+idx);
                push((int64_t)val);
                break;
            }
            case OP_SLOAD: {
                size_t idx=(uint32_t)v;
                int16_t val=*(int16_t *)cv_eptr(&vars, bp+idx);
                push((int64_t)val);
                break;
            }
            case OP_ILOAD: {
                size_t idx=(uint32_t)v;
                int32_t val=*(int32_t *)cv_eptr(&vars, bp+idx);
                push((int64_t)val);
                break;
            }
            case OP_LLOAD: {
                size_t idx=(uint32_t)v;
                int64_t val=*(int64_t *)cv_eptr(&vars, bp+idx);
                push(val);
                break;
            }
            case OP_ALOAD: {
                size_t idx=(uint32_t)v;
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
            case OP_CADD: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(int8_t)(g+h)); break; }
            case OP_CSUB: { cv_pop(&stack,&h); cv_pop(&stack,&g); push((int64_t)(int8_t)(g-h)); break; }
            case OP_CMUL: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(int8_t)(g*h)); break; }
            case OP_CDIV: { cv_pop(&stack,&h); cv_pop(&stack,&g); if(h==0) error("CDIV: division by zero"); push((int64_t)(int8_t)(g/h)); break; }
            case OP_CMOD: { cv_pop(&stack,&h); cv_pop(&stack,&g); if(!h) error("CMOD: division by zero"); push((int64_t)(int8_t)(g%h)); break; }
            case OP_SADD: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(int16_t)(g+h)); break; }
            case OP_SSUB: { cv_pop(&stack,&h); cv_pop(&stack,&g); push((int64_t)(int16_t)(g-h)); break; }
            case OP_SMUL: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(int16_t)(g*h)); break; }
            case OP_SDIV: { cv_pop(&stack,&h); cv_pop(&stack,&g); if(h==0) error("SDIV: division by zero"); push((int64_t)(int16_t)(g/h)); break; }
            case OP_SMOD: { cv_pop(&stack,&h); cv_pop(&stack,&g); if(!h) error("SMOD: division by zero"); push((int64_t)(int16_t)(g%h)); break; }
            case OP_IADD: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(int32_t)(g+h)); break; }
            case OP_ISUB: { cv_pop(&stack,&h); cv_pop(&stack,&g); push((int64_t)(int32_t)(g-h)); break; }
            case OP_IMUL: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(int32_t)(g*h)); break; }
            case OP_IDIV: { cv_pop(&stack,&h); cv_pop(&stack,&g); if(h==0) error("IDIV: division by zero"); push((int64_t)(int32_t)(g/h)); break; }
            case OP_IMOD: { cv_pop(&stack,&h); cv_pop(&stack,&g); if(!h) error("IMOD: division by zero"); push((int64_t)(int32_t)(g%h)); break; }
            case OP_LADD: { cv_pop(&stack,&g); cv_pop(&stack,&h); push(g+h); break; }
            case OP_LSUB: { cv_pop(&stack,&h); cv_pop(&stack,&g); push(g-h); break; }
            case OP_LMUL: { cv_pop(&stack,&g); cv_pop(&stack,&h); push(g*h); break; }
            case OP_LDIV: { cv_pop(&stack,&h); cv_pop(&stack,&g); if(h==0) error("LDIV: division by zero"); push(g/h); break; }
            case OP_LMOD: { cv_pop(&stack,&h); cv_pop(&stack,&g); if(!h) error("LMOD: division by zero"); push(g%h); break; }
            case OP_AADD: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((uint64_t)g+(uint64_t)h)); break; }
            case OP_ASUB: { cv_pop(&stack,&h); cv_pop(&stack,&g); push((int64_t)((uint64_t)g-(uint64_t)h)); break; }
            case OP_CEQ: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((int8_t)g==(int8_t)h)); break; }
            case OP_CNEQ: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((int8_t)g!=(int8_t)h)); break; }
            case OP_CLT: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((int8_t)h<(int8_t)g)); break; }
            case OP_CBT: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((int8_t)h>(int8_t)g)); break; }
            case OP_CLE: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((int8_t)h<=(int8_t)g)); break; }
            case OP_CBE: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((int8_t)h>=(int8_t)g)); break; }
            case OP_SEQ: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((int16_t)g==(int16_t)h)); break; }
            case OP_SNEQ: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((int16_t)g!=(int16_t)h)); break; }
            case OP_SLT: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((int16_t)h<(int16_t)g)); break; }
            case OP_SBT: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((int16_t)h>(int16_t)g)); break; }
            case OP_SLE: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((int16_t)h<=(int16_t)g)); break; }
            case OP_SBE: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((int16_t)h>=(int16_t)g)); break; }
            case OP_IEQ: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((int32_t)g==(int32_t)h)); break; }
            case OP_INEQ: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((int32_t)g!=(int32_t)h)); break; }
            case OP_ILT: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((int32_t)h<(int32_t)g)); break; }
            case OP_IBT: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((int32_t)h>(int32_t)g)); break; }
            case OP_ILE: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((int32_t)h<=(int32_t)g)); break; }
            case OP_IBE: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((int32_t)h>=(int32_t)g)); break; }
            case OP_LEQ: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(h==g)); break; }
            case OP_LNEQ: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(h!=g)); break; }
            case OP_LLT: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(h<g)); break; }
            case OP_LBT: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(h>g)); break; }
            case OP_LLE: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(h<=g)); break; }
            case OP_LBE: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(h>=g)); break; }
            case OP_AEQ: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((uint64_t)h==(uint64_t)g)); break; }
            case OP_ANEQ: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((uint64_t)h!=(uint64_t)g)); break; }
            case OP_ALT: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((uint64_t)h<(uint64_t)g)); break; }
            case OP_ABT: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((uint64_t)h>(uint64_t)g)); break; }
            case OP_ALE: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((uint64_t)h<=(uint64_t)g)); break; }
            case OP_ABE: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((uint64_t)h>=(uint64_t)g)); break; }
            case OP_CAND: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(int8_t)((int8_t)g&(int8_t)h)); break; }
            case OP_COR: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(int8_t)((int8_t)g|(int8_t)h)); break; }
            case OP_CXOR: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(int8_t)((int8_t)g^(int8_t)h)); break; }
            case OP_CNOT: { cv_pop(&stack,&g); push((int64_t)(int8_t)(~(int8_t)g)); break; }
            case OP_SAND: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(int16_t)((int16_t)g&(int16_t)h)); break; }
            case OP_SOR: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(int16_t)((int16_t)g|(int16_t)h)); break; }
            case OP_SXOR: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(int16_t)((int16_t)g^(int16_t)h)); break; }
            case OP_SNOT: { cv_pop(&stack,&g); push((int64_t)(int16_t)(~(int16_t)g)); break; }
            case OP_IAND: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(int32_t)((int32_t)g&(int32_t)h)); break; }
            case OP_IOR: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(int32_t)((int32_t)g|(int32_t)h)); break; }
            case OP_IXOR: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(int32_t)((int32_t)g^(int32_t)h)); break; }
            case OP_INOT: { cv_pop(&stack,&g); push((int64_t)(int32_t)(~(int32_t)g)); break; }
            case OP_LAND: { cv_pop(&stack,&g); cv_pop(&stack,&h); push(g&h); break; }
            case OP_LOR: { cv_pop(&stack,&g); cv_pop(&stack,&h); push(g|h); break; }
            case OP_LXOR: { cv_pop(&stack,&g); cv_pop(&stack,&h); push(g^h); break; }
            case OP_LNOT: { cv_pop(&stack,&g); push(~g); break; }
            case OP_AAND: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((uint64_t)g&(uint64_t)h)); break; }
            case OP_AOR: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((uint64_t)g|(uint64_t)h)); break; }
            case OP_AXOR: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((uint64_t)g^(uint64_t)h)); break; }
            case OP_ANOT: { cv_pop(&stack,&g); push((int64_t)(~(uint64_t)g)); break; }
            case OP_CSHL: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(int8_t)((int8_t)g<<(h&7))); break; }
            case OP_CSHR: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(int8_t)((int8_t)g>>(h&7))); break; }
            case OP_SSHL: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(int16_t)((int16_t)g<<(h&15))); break; }
            case OP_SSHR: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(int16_t)((int16_t)g>>(h&15))); break; }
            case OP_ISHL: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(int32_t)((int32_t)g<<(h&31))); break; }
            case OP_ISHR: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)(int32_t)((int32_t)g>>(h&31))); break; }
            case OP_LSHL: { cv_pop(&stack,&g); cv_pop(&stack,&h); push(g<<(h&63)); break; }
            case OP_LSHR: { cv_pop(&stack,&g); cv_pop(&stack,&h); push(g>>(h&63)); break; }
            case OP_ASHL: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((uint64_t)g<<(h&63))); break; }
            case OP_ASHR: { cv_pop(&stack,&g); cv_pop(&stack,&h); push((int64_t)((uint64_t)g>>(h&63))); break; }
            case OP_CNEG: { cv_pop(&stack,&g); push((int64_t)(int8_t)(-(int8_t)g)); break; }
            case OP_SNEG: { cv_pop(&stack,&g); push((int64_t)(int16_t)(-(int16_t)g)); break; }
            case OP_INEG: { cv_pop(&stack,&g); push((int64_t)(int32_t)(-(int32_t)g)); break; }
            case OP_LNEG: { cv_pop(&stack,&g); push(-g); break; }
            case OP_ANEG: { cv_pop(&stack,&g); push((int64_t)(-(uint64_t)g)); break; }
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
                printf("ip=%zx stack: s=%zx c=%zx es=%zx d=%p\n", P-current_module->code, stack.s, stack.c, stack.es, (void*)stack.d);
                printf("stack: ");
                for(i=0; i<stack.s; i++)
                    printf("%lx ", *((int64_t *)cv_eptr(&stack, i)));
                puts("<< Stack Pointer\r\n(stack dump end)");
                abort();
            }
        }
        P++;
    }

    if(debug) printf("Ended on instruction pointer %llx\r\n", i);

    cv_free(&stack);
    cv_free(&vars);
    cv_free(&frames);
    cv_free(&args);
    free(vm_state.heap);
    return 0;
}
