#include "parser.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>

static Scope nullscope;

void par_init(Parser *n, const ds fni, const ds fno, bool deb) {
    printf("  memset...\n");
    memset(n, 0, sizeof(Parser));
    printf("  cv_init code...\n");
    cv_init(&n->code, 256, sizeof(unsigned char));
    printf("  init (malloc) heap...\n");
    n->hp=0;
    n->heap=malloc(n->hs=1024);
    printf("  cv_init file...\n");
    cv_init(&n->file, 16, sizeof(Token));
    printf("  cv_init imports...\n");
    cv_init(&n->imports, 4, sizeof(Import));
    printf("  scos_init...\n");
    scos_init(&n->scopes);
    scos_es(&n->scopes);

    printf("  getenv...\n");
    char *env=getenv("CHKALOV");
    printf("  env ptr = %p\n", (void*)env);
    if(!env) {
        printf("  CHKALOV is NULL!\n");
        return;
    }
    printf("  env = '%s'\n", env);

    printf("  strdup...\n");
    ds yyy=strdup(env);
    printf("  env_copy = '%s'\n", yyy);
    ds_cat(&yyy, "/imports-table.txt", NULL);
    printf("  yyy = '%s'\n", yyy);
    n->fn=strdup(fno);
    // ...
    FILE *a=fopen(yyy, "r");
    if(!a) error("Imports table not found!");
    char sa[128], section[128];
    fgets(sa, sizeof(sa), a);
    if(strncmp(sa, "$DOCTYPE CHKALOV IMPORTS <1.00>", 4)) error("Imports table (imports-table.txt) corrupted!");
    while(!feof(a)) {
        fgets(sa, sizeof(sa), a);
        if(sa[0]=='%') {
            sa[strcspn(sa, "\n")]=0;
            strcpy(section, sa+1);
            Import imp;
            imp.name=strdup(sa+1);
            cv_init(&imp.funcs, 4, sizeof(ds));
            cv_push(&n->imports, &imp);
        } else if(sa[0]=='+') {
            sa[strcspn(sa, "\n")]=0;
            for(uint64_t i=0ULL; i<n->imports.s; i++) {
                Import *aa=(Import *)cv_eptr(&n->imports, i);
                if(SEQU(section, aa->name)) {
                    imp_add(aa, strdup(sa+1));
                }
            }
        };
    }
    fclose(a);
    a=fopen(fni, "rb");
    ds x;
    while(!feof(a)) {
        ds_gl(&x, a);
        par_par(n, x);
    }
    fclose(a);
}

void par_render(Parser *a, Vm vm) {
    printf("par_render: opcode=%x type=%x value=%llx\n", vm.opcode, vm.type, (long long)vm.value);
    unsigned char op=vm.opcode, ty=vm.type;
    cv_push(&a->code, &op);
    cv_push(&a->code, &ty);
    size_t sz=ty&0x0f;
    int64_t v=vm.value;
    unsigned char *p=(unsigned char*)&v;
    for(size_t i=0ULL;i<sz;i++) cv_push(&a->code, &p[i]);
    a->lit=vm.type;
}

void par_free(Parser *z) {
    /*par_render(z, (Vm){ALLOC, PTR|0x01, 8});
    par_render(z, (Vm){STORE, IDX|0x00, 0});
    par_render(z, (Vm){PUSH, INT|0x01, 52});
    par_render(z, (Vm){LOAD, IDX|0x00, 0});
    par_render(z, (Vm){SETFIELD, PTR|0x00});
    par_render(z, (Vm){LOAD, IDX|0x00, 0});
    par_render(z, (Vm){GETFIELD, IDX|0x01, 0x00});
    par_render(z, (Vm){CALL, 0x00, 0x00});*/

    /*смещение выделенного лежит на стеке.
    Уже.
    А вот значение, мы передаём в сетфиелде.*/

    /* a вот с гетфилдом, наоборот.
     * Смещение мы кладём ему в аргументы, а основной пойнтер на стеке.
     * Компилер видит класс:
     * class Point {
    Int x
    Int y
    }

    alloc 8
    store 0
    push 52
    load 0
    setfield 0
    load 0
    getfield 0
    call 0

    var p: Point
    p.x=52
    var a=p.x; // Int
    и
    alloc 8
    store 0
    load 0
    push 52
    setfield 0
    load 0
    getfield


    */

    printf("par_free: fn = '%s'\n", z->fn);
    FILE *fp = fopen(z->fn, "wb");
    printf("par_free: fp = %p\n", (void*)fp);
    uint32_t magic=0x05020100;
    uint64_t size=0, i, a, b, c; // 4 + 5 + 5 = 14 !!!!!
    fwrite(&magic, 1, sizeof(uint32_t), fp); ///// 0 2 1 5     9 0 0 0 0 10 0 0 0 0
    puts("writing heap...");
    size=z->hp;
    fwrite(&size, 1, sz(size), fp);
    fwrite(z->heap, 1, size, fp);
    size=z->code.s;
    fwrite(&size, 1, sz(size), fp);
    fwrite(z->code.d, 1, size, fp);
    fclose(fp);
    free(z->heap);
    cv_free(&z->code);
    Token *fip;
    for(i=0; i<z->file.s; i++) {
        fip=cv_eptr(&z->file, i);
        free(fip->value);
    }
    cv_free(&z->file);
}

void par_par(Parser *a, const ds l) {
    Token t;
    uint64_t pos=0;
    while(pos<strlen(l)) {
        t.value=NULL;
        while(pos<strlen(l)&&isspace(l[pos])) pos++;

        if(pos>=strlen(l)) {
            Token abcv={EOF, strdup("")};
            cv_push(&a->file, &abcv);
        }

        IFCS(l+pos, "class", 5) {
            pos+=5;
            t=(Token){CLASS, strdup("class")};
            cv_push(&a->file, &t);
            continue;
        }
        IFCS(l+pos, "var", 3) {
            pos+=3;
            t=(Token){VAR, strdup("var")};
            cv_push(&a->file, &t);
            continue;
        }
        IFCS(l+pos, "==", 2) {
            pos+=2;
            t=(Token){EQ, strdup("==")};
            cv_push(&a->file, &t);
            continue;
        }
        IFCS(l+pos, "!=", 2) {
            pos+=2;
            t=(Token){NEQ, strdup("!=")};
            cv_push(&a->file, &t);
            continue;
        }
        IFCS(l+pos, "if", 2) {
            pos+=2;
            t=(Token){IF, strdup("if")};
            cv_push(&a->file, &t);
            continue;
        }
        if(isalpha(l[pos])) { // ID
            size_t start=pos;
            while(pos<strlen(l)&&isalnum(l[pos])) pos++;
            t.type=ID;
            t.value=NULL;
            ds_sub(&t.value, l, start, pos-start);
            cv_push(&a->file, &t);
            continue;
        }
        if(isdigit(l[pos])) { // число
            size_t start=pos;
            while(pos<strlen(l)&&isdigit(l[pos])) pos++;
            t.type=NUMBER;
            t.value=NULL;
            ds_sub(&t.value, l, start, pos-start);
            cv_push(&a->file, &t);
            continue;
        }
        if(l[pos]=='\"') { // строка
            size_t start=++pos;
            while(pos<strlen(l)&&l[pos]!='\"') pos++;
            t.type=STRING;
            t.value=NULL;
            t.value=malloc(pos-start);
            size_t i, sp=0;
            for(i=start; i<pos; i++, sp++) {
                if(l[i]=='\\') {
                    i++;
                    switch(l[i]) {
                        case 'n': t.value[sp++]='\n'; break;
                        case 'b': t.value[sp++]='\b'; break;
                        case 'r': t.value[sp++]='\r'; break;
                        case 'a': t.value[sp++]='\a'; break;
                        case '\\': t.value[sp++]='\\'; break;
                        default: i--; break;
                    }
                    i++;
                }
                t.value[sp]=l[i];
            }
            pos++;
            //ds_sub(&t.value, l, start, pos++-start);
            cv_push(&a->file, &t);
            continue;
        }
        char c=l[pos++];
        switch(c) {
            case '=':
                t=(Token){ASSIGN, strdup("=")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case '+':
                t=(Token){PLUS, strdup("+")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case '-':
                t=(Token){MINUS, strdup("-")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case '*':
                t=(Token){STAR, strdup("*")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case '/':
                t=(Token){SLASH, strdup("/")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case '{':
                t=(Token){LPAREN, strdup("{")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case '}':
                t=(Token){RPAREN, strdup("}")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case ':':
                t=(Token){COLON, strdup(":")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case '@':
                t=(Token){LCALL, strdup("@")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case '(':
                t=(Token){LBRACE, strdup("(")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case ')':
                t=(Token){RBRACE, strdup(")")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case ',':
                t=(Token){COMMA, strdup(",")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case '.':
                t=(Token){DOT, strdup(".")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
        }
    }
}

Token par_next(Parser *a) {
    return *((Token *)cv_eptr(&a->file, a->p));
}

Token par_nexti(Parser *a) {
    return *((Token *)cv_eptr(&a->file, ++a->p));
}

void par_parIns(Parser *a) {
    Token t;
    t=par_next(a);
    switch(t.type) {
        case LPAREN: {
            scos_es(&a->scopes);
            par_parBlock(a);
            break;
        }
        case RPAREN: {
            scos_ex(&a->scopes);
            break;
        }
        case LCALL: {
            Token method=par_nexti(a);
            if(method.type!=ID) error("Expected name function!");
            Token ntam=par_nexti(a);
            if(ntam.type!=LBRACE) error("expected '('!");
            a->p++;
            par_parExpr(a);
            if(par_next(a).type!=RBRACE) error("expected )!");
            for(uint64_t i=0; i<a->imports.s; i++) {
                Import ppp=*((Import *)cv_eptr(&a->imports, i));
                if(imp_cont(&ppp, method.value)) {
                    ds aaa=strdup(ppp.name);
                    ds_cat(&aaa, ".", method.value, NULL);
                    size_t k=par_heapIns(a, aaa);
                    par_render(a, (Vm){CALL, LIBRARY|selszu(k), k});
                    break;
                }
            }
            break;
        }
        case FUN: {
            Token funcname=par_nexti(a);
            if(par_nexti(a).type!=LPAREN) error("Expected {!");
            break;
        }
        case VAR: {
            Token vt=par_nexti(a);
            Token name;
            char type=par_stt(vt.value);
            if(par_nexti(a).type!=COLON) error("Expected ':' !");
            do {
                //if(debug) cout << "VAR loop: p=" << p << " token=" << par_next(a) << " type=" << file[p].type << endl;
                name=par_nexti(a);
                if(name.type!=ID) error("Expected variable name!");
                scos_addv(&a->scopes, name.value, type);
                par_render(a, (Vm){PUSH, type|0, 0});
                par_render(a, (Vm){STORE, IDX|selsz(sco_ggi()), sco_ggi()});
                //if(debug) cout << "Var: " << name.value << " type: " << vt.value << endl;
                a->p++;
            } while(a->p<a->file.s&&par_next(a).type==COMMA);
            a->p--;
            break;
        }
        case ID: {
            Token ja=par_nexti(a);
            if(ja.type!=ASSIGN) error("case ID: expected '=', detected '%s'", ja.value);
            a->p++;
            par_parExpr(a);
            a->p--;
            variable vaa;
            scos_gv(&a->scopes, t.value, &vaa);
            if((!(ISNUM(vaa.type)&&ISNUM(a->lit)))&&(!(ISSTR(vaa.type)&&ISSTR(a->lit)))) error("types mismatch! vaa.type=%x, lit=%x!", vaa.type, a->lit);
            par_render(a, (Vm){STORE, IDX|selsz(vaa.id), vaa.id});
            break;
        }
        case IF: {
            par_parIf(a);
            break;
        }
        case CLASS: {

        }
        case ENOF: {
            break;
        }
        case NOT: {
            break;
        }
        default: a->p++;
    }
}

void par_parFile(Parser *a) {
    for(a->p=0; a->p<a->file.s; a->p++) {
        printf("par_parFile: p=%zu, type=%d\n", a->p, ((Token*)cv_eptr(&a->file, a->p))->type);
        par_parIns(a);
    }
};

char par_stt(const ds s) {
    if(SEQU(s, "Int")) return INT;
    if(SEQU(s, "Xshort")) return XSHORT;
    if(SEQU(s, "Char")) return CHAR;
    if(SEQU(s, "String")) return STR;
    if(SEQU(s, "Long")) return LONG;
    return 0;
}

bool imp_cont(Import *a, ds b) {
    for(uint64_t i=0; i<a->funcs.s; i++) {
        if(SEQU(*(ds*)cv_eptr(&a->funcs, i), b)) return true;
    }
    return false;
}
void imp_add(Import *a, ds z) {
    ds copy=strdup(z);
    cv_push(&a->funcs, &copy);
}

void par_parIf(Parser *a){
    if(par_nexti(a).type!=LBRACE) {
        error("Expected '(' after if!");
    }
    scos_es(&a->scopes);
    a->p++;
    par_parExpr(a);
    Token y=par_next(a);
    printf("y value: %s \r\n", y.value);
    a->p++;
    par_parExpr(a);
    a->p++;
    printf("value: %s \r\n", par_next(a).value);
    switch(y.type) {
        case EQ: {
            uint64_t cs=a->code.s;
            char *cd;
            par_render(a, (Vm){IFNE, PTR|8, 0x00});
            puts("before parBlock");
            par_parBlock(a);
            uint64_t tar=0, shift=8-selszu(tar);
            memmove(a->code.d+cs+2+selszu(tar), a->code.d+cs+10, a->code.s-(cs+10));
            a->code.s-=shift;
            printf("shift: %llx \r\n", shift);
            memcpy(a->code.d+cs+2, &tar, selszu(tar));
            cd=((char*)a->code.d);
            cd[cs+1]=(cd[cs+1]&0xF0)|selszu(tar);
            break;
        }
    }
}

void par_parBlock(Parser *a) {
    a->p++;
    while(a->p<a->file.s&&par_next(a).type!=RPAREN) {
        printf("a->p: %llu value: '%s'", a->p, par_next(a).value);
        par_parIns(a);
    }
    //p++;
    //scopes.exitScope();
}

void par_parExpr(Parser *a) {
    Token au;
    par_parTerm(a);
    while(a->p<a->file.s) {
        au=par_next(a);
        if(au.type==PLUS) {
            a->p++;
            par_parTerm(a);
            par_render(a, (Vm){ADD});
        } else if(au.type==MINUS) {
            a->p++;
            par_parTerm(a);
            par_render(a, (Vm){SUB});
        } else break;
    }
}

void par_parTerm(Parser *a) {
    Token au;
    par_parFact(a);
    while(a->p<a->file.s) {
        au=par_next(a);
        if(au.type==STAR) {
            a->p++;
            par_parFact(a);
            par_render(a, (Vm){MUL});
        } else if(au.type==SLASH) {
            a->p++;
            par_parFact(a);
            par_render(a, (Vm){DIV});
        } else break;
    }
}

size_t par_heapIns(Parser *a, ds k) {
    if(strlen(k)>=a->hs) a->heap=realloc(a->heap, a->hs=strlen(k)<<1);
    memcpy(a->heap+a->hp, k, strlen(k)+1);
    size_t r=a->hp;
    a->hp+=strlen(k)+1;
    return r;
}

void par_parFact(Parser *a) {
    Token au=par_next(a);

    if(au.type==NUMBER) {
        a->p++;
        par_render(a, (Vm){PUSH, LONG|selsz(atoll(au.value)), atoll(au.value)});
    }
    else if(au.type==STRING) {
        a->p++;
        size_t k=par_heapIns(a, au.value);
        par_render(a, (Vm){PUSH, STR|selszu(k), k});
    }
    else if(au.type==LBRACE) {
        a->p++;
        par_parExpr(a);
        if(par_next(a).type!=RBRACE) error("Expected ')'!");
        Token *aap=(Token *)cv_eptr(&a->file, a->p+1);
        if(a->p+1<a->file.s&&(aap->type==STAR||aap->type==SLASH)) {
            a->p++;
        }
    }
   else if(au.type==ID) {
        a->p++;
        ds name=au.value;

        if(a->p<a->file.s&&((Token *)cv_eptr(&a->file, a->p))->type==LBRACE) {
        } else {
            variable var;
            scos_gv(&a->scopes, name, &var);
            if(!var.name) error("Undeclared variable \'%s\'", name);
            par_render(a, (Vm){LOAD, IDX|selsz(var.id), var.id});
        }
    }
    else if(au.type==LCALL) {
        Token method=par_nexti(a);
        if(method.type!=ID) error("Expected name function!");
        Token ntam=par_nexti(a);
        if(ntam.type!=LBRACE) error("expected '('!");
        a->p++;
        par_parExpr(a);
        if(par_next(a).type!=RBRACE) error("expected )!");
        for(uint64_t i=0; i<a->imports.s; i++) {
            Import ppp=*((Import *)cv_eptr(&a->imports, i));
            if(imp_cont(&ppp, method.value)) {
                ds aaa=strdup(ppp.name);
                ds_cat(&aaa, ".", method.value, NULL);
                size_t k=par_heapIns(a, aaa);
                par_render(a, (Vm){CALL, LIBRARY|selszu(k), k});
                break;
            }
        }
    }
    else {
        error("ParseFactor: Unexpected token \'%s\'!", au.value);
    }
}
