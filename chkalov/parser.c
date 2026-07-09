#include "parser.h"
#include "token.h"
#include <stdio.h>
//#include <сstdlib>
#include <stdlib.h>

static Scope nullscope;

uint8_t eee(TokenType t) {
    switch(t) {
        case STRING: return STR;
        case NUMBER: return LONG;
        default:     return NOL;
    }
}

//const char* envi = getenv("CHKALOV");
void par_init(Parser *n, const ds fni, const ds fno, bool deb) {
    printf("  memset...\n");
    memset(n, 0, sizeof(Parser));
    printf("  cv_init code...\n");
    cv_init(&n->code, 256, sizeof(unsigned char));
    printf("  cv_init pool...\n");
    cv_init(&n->pool, 8, sizeof(Pool));
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
    /*par_render(z, (Vm){ALLOC, PTR|0x04, 8});
    par_render(z, (Vm){SETFIELD, 0x00});
    par_render(z, (Vm){PUSH, INT|0x01, 0x52});
    par_render(z, (Vm){GETFIELD, 0x00});*/

    /*смещение выделенного лежит на стеке.
    Уже.
    А вот значение, мы передаём в сетфиелде.*/

    /* a вот с гетфилдом, наоборот.
     * Смещение мы кладём ему в аргументы, а значение на стеке.
     * Компилер видит класс:
     * struct a {
    Int a
    Int b
    }

    */

    printf("par_free: fn = '%s'\n", z->fn);
    FILE *fp = fopen(z->fn, "wb");
    printf("par_free: fp = %p\n", (void*)fp);
    uint32_t magic=0x05020100;
    uint64_t size=0, i, a, b, c; // 4 + 5 + 5 = 14 !!!!!
    fwrite(&magic, 1, sizeof(uint32_t), fp); ///// 0 2 1 5     9 0 0 0 0 10 0 0 0 0
    size=z->pool.s;
    Pool *p;
    fwrite(&size, 1, sz(size), fp);
    puts("writing pool...");
    for(i=0; i<size; i++) {
        p=(Pool*)cv_eptr(&z->pool, i);
        printf("Iteration %i, pool value = '%s'\r\n", i, p->value);
        fwrite(&p->type, 1, sz(char), fp);
        puts("writed.");
        a=strlen(p->value);
        fwrite(&a, 1, sz(a), fp);
        fwrite(p->value, 1, a, fp);
        free(p->value);
    }
    size=z->code.s;
    fwrite(&size, 1, sz(size), fp);
    fwrite(z->code.d, 1, size, fp);
    fclose(fp);
    cv_free(&z->pool);
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
                    Pool ibm=(Pool){LIBRARY, aaa};
                    cv_push(&a->pool, &ibm);
                    par_render(a, (Vm){CALL, LIBRARY|selsz(a->pool.s-1), a->pool.s-1});
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
            if(ja.type!=ASSIGN) error("expected '='!");
            a->p++;
            par_parExpr(a);
            a->p--;
            variable vaa;
            scos_gv(&a->scopes, t.value, &vaa);
            if((!(ISNUM(vaa.type)&&ISNUM(a->lit)))&&(!(ISSTR(vaa.type)&&ISSTR(a->lit)))) error("types mismatch!");
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
        //default: p++;
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
    return NOL;
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
/*void Parser::parseIf() {
    if(file[++p].type!=LBRACE) {
        error("Expected ( after if!");
    }
    scopes.enterScope();
    p++;
    Token x=file[p++], y=file[p++], z=file[p++];
    if(debug) cout << "x: " << x.value << " y: " << y.value << " z: "  << z.value << '!' << endl;
    Vm u, o;
    if(debug) cout << "ok" << endl;
    if(x.type==ID) u={LOAD, seltypeu(scopes.getVar(x.value).id), scopes.getVar(x.value).id};
    else u={PUSH, seltype(atoll(x.value.c_str())), atoll(x.value.c_str())};
    if(z.type==ID) o={LOAD, seltypeu(scopes.getVar(x.value).id), scopes.getVar(x.value).id};
    else o={PUSH, seltype(atoll(z.value.c_str())), atoll(z.value.c_str())};
    switch(y.type) {
        case EQ: {
            render(u);
            render(o);
            uint64_t cs=code.size();
            if(debug) cout << "size: " << code.size() << endl;
            render({IFNE, ADDR, 0});
            if(debug) cout << " CS: " << cs << '!' << endl;
            parseBlock();
            if(debug) cout << " code.size(): " << code.size() << '!' << endl;
            uint64_t tar=code.size();
            cout << "PATCH: cs=" << cs << " cs+2=" << cs+2 << " tar=" << tar << dec << " (0x" << hex << tar << ")" << endl;
            memcpy(code.data()+cs+2, &tar, sz(tar));
            break;
        }
//        case NEQ: {
//            //render({PUSH, seltype(atoll(x.value.c_str()))});
//            //render({PUSH, seltype(atoll(z.value.c_str()))});
//            render({IFE, ADDR, 0});
//            uint64_t cs=code.size()-1;
//            if(debug) cout << " CS: " << cs << '!' << endl;
//            parseBlock();
//            if(debug) cout << " code.size(): " << code.size() << '!' << endl;
//            break;
//        }
    }
}*/

void par_parIf(Parser *a){};

void par_parBlock(Parser *a) {
    a->p++;
    while(a->p<a->file.s&&par_next(a).type!=RPAREN) {
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

void par_parFact(Parser *a) {
    Token au=par_next(a);

    if(au.type==NUMBER) {
        a->p++;
        par_render(a, (Vm){PUSH, LONG|selsz(atoll(au.value)), atoll(au.value)});
    }
    else if(au.type==STRING) {
        a->p++;
        Pool at=(Pool){STR, strdup(au.value)};
        cv_push(&a->pool, &at);
        par_render(a, (Vm){PUSH, selstrt(a->pool.s-1), a->pool.s-1});
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
            if(!var.name) error("Undefined variable: \'%s\', %llx", name, var.id);
            par_render(a, (Vm){LOAD, IDX|selsz(var.id), var.id});
        }
    }
    else {
        error("ParseFactor: Unexpected token \'%s\'!", au.value);
    }
}
