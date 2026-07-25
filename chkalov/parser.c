#include "parser.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>

static Scope nullscope;

void par_parFile(Parser *a) {
    AstNode *an;
    a->p=0;
    while(a->p<a->file.s) {
        an=par_par_expr(a);
        if(an) ast_add((AstNode *)a->root, an);
    }
}

void codegen(AstNode *node, Parser *a) {
    if(!node) return;
    switch(node->type) {
        case AST_STMT_BLOCK: {
            AstStmtBlock *block=(AstStmtBlock *)node;
            for(size_t i=0; i<block->count; i++) {
                codegen(block->stats[i], a);
            }
            break;
        }
        case AST_STMT_CALL: {
            size_t j;
            AstStmtCall *call=(AstStmtCall *)node;
            codegen(call->arg, a);
            char *method=call->name;
            for(size_t i=0; i<a->imports.s; i++) {
                Import ppp=*((Import *)cv_eptr(&a->imports, i));
                if(imp_cont(&ppp, method)) {
                    ds aaa=strdup(ppp.name);
                    ds_cat(&aaa, ".", method, NULL);
                    j=par_heapIns(a, aaa);
                    par_render(a, CALL, STR|selszu(j), j);
                    break;
                }
            }
            if(a->debug) puts("call");
            break;
        }
        case AST_EXPR_LITERAL: {
            long long j;
            AstExprLiteral *lit=(AstExprLiteral *)node;
            switch(lit->type) {
                case NUMBER: {
                    j=atoll(lit->value);
                    par_render(a, PUSH, LONG|selsz(j), j);
                    if(a->debug) puts("push long");
                    break;
                }
                case STRING: {
                    j=par_heapIns(a, lit->value);
                    par_render(a, PUSH, STR|selszu(j), j);
                    if(a->debug) puts("push string");
                    break;
                }
            }
            break;
        }
        case AST_BINARY: {
            AstBinary *bin=(AstBinary *)node;
            codegen(bin->left, a);
            codegen(bin->right, a);
            switch(bin->op) {
                case PLUS: par_render(a, ADD, 0, 0); break;
                case MINUS: par_render(a, SUB, 0, 0); break;
                case STAR: par_render(a, MUL, 0, 0); break;
                case SLASH: par_render(a, DIV, 0, 0); break;
                default: error("Unknown binary operator: %d", bin->op);
            }
            break;
        }
    }
}

void expect(Parser *a, TokenType t, char *s) {
    if(par_this(a)->type==t) a->p++;
    else error("Expected %s, detected: %s", s, par_this(a)->value);
}

Token *par_this(Parser *a) {
    return (Token *)cv_eptr(&a->file, a->p);
}
Token *par_next(Parser *a) {
    return (Token *)cv_eptr(&a->file, ++a->p);
}

void par_init(Parser *n, const ds fni, const ds fno, int deb) {
    memset(n, 0, sizeof(Parser));
    cv_init(&n->code, 256, sizeof(unsigned char));
    n->hp=0;
    n->heap=malloc(n->hs=1024);
    n->root=ast_create_block();
    cv_init(&n->imports, 4, sizeof(Import));
    scos_init(&n->scopes);
    scos_es(&n->scopes);
    cv_init(&n->file, 16, sizeof(Token));
    n->debug=deb;
    char *env=getenv("CHKALOV");
    if(n->debug) printf("  env ptr = %p\n", (void*)env);
    if(!env) {
        printf("  CHKALOV is NULL!\n");
        return;
    }
    ds yyy=strdup(env);
    ds_cat(&yyy, "/imports-table.txt", NULL);
    if(n->debug) printf("  yyy = '%s'\n", yyy);
    n->fn=strdup(fno);
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
        par_lexer(n, x);
    }
    fclose(a);
}

void par_render(Parser *a, uint8_t op, uint8_t ty, int64_t v) {
    if(a->debug) printf("par_render: opcode=%x type=%x value=%llx\n", op, ty, v);
    cv_push(&a->code, &op);
    cv_push(&a->code, &ty);
    size_t sz=ty&0x0f;
    unsigned char *p=(unsigned char*)&v;
    for(size_t i=0ULL;i<sz;i++) cv_push(&a->code, &p[i]);
}

void par_free(Parser *z) {
    if(z->debug) printf("par_free: fn = '%s'\n", z->fn);
    FILE *fp = fopen(z->fn, "wb");
    if(z->debug) printf("par_free: fp = %p\n", (void*)fp);
    uint32_t magic=0x05020100;
    uint64_t size=0, i, a, b, c; // 4 + 5 + 5 = 14 !!!!!
    fwrite(&magic, 1, sizeof(uint32_t), fp); ///// 0 2 1 5     9 0 0 0 0 10 0 0 0 0
    if(z->debug) puts("writing heap...");
    size=z->hp;
    fwrite(&size, 1, sz(size), fp);
    fwrite(z->heap, 1, size, fp);
    size=z->code.s;
    fwrite(&size, 1, sz(size), fp);
    fwrite(z->code.d, 1, size, fp);
    fclose(fp);
    free(z->heap);
    cv_free(&z->code);
    ast_free((AstNode *)z->root);
}

size_t par_heapIns(Parser *a, ds k) {
    if(strlen(k)+a->hp>=a->hs) a->heap=realloc(a->heap, a->hs=strlen(k)<<1);
    memcpy(a->heap+a->hp, k, strlen(k)+1);
    size_t r=a->hp;
    a->hp+=strlen(k)+1;
    return r;
}

void par_lexer(Parser *a, ds l) {
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
        if(isdigit(l[pos])) {
            size_t start=pos;
            while(pos<strlen(l)&&isdigit(l[pos])) pos++;
            t.type=NUMBER;
            t.value=NULL;
            ds_sub(&t.value, l, start, pos-start);
            cv_push(&a->file, &t);
            continue;
        }
        if(l[pos]=='\"') {
            size_t start=++pos;
            while(pos<strlen(l)&&l[pos]!='\"') pos++;
            t.type=STRING;
            t.value=NULL;
            t.value=malloc((pos-start)+1);
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
            t.value[sp]=0;
            pos++;
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

AstNode *par_par_primary(Parser *a) {
    Token t=*par_this(a);
    AstNode *arg;
    if(a->debug) puts("par primary start");
    a->p++;
    switch(t.type) {
        case NUMBER: return (AstNode *)ast_create_literal(NUMBER, t.value);
        case LBRACE: {
            AstNode *p;
            p=par_par_expr(a);
            expect(a, RBRACE, "')'");
            return p;
        }
        case STRING: return(AstNode *)ast_create_literal(STRING, t.value);
    }
    return NULL;
}

AstNode *par_par_term(Parser *a) {
    Token op;
    if(a->debug) puts("par term start");
    AstNode *l=par_par_primary(a);
    if(a->debug) printf("after calling primary (in term): p=%llu token='%s'\n", a->p, par_this(a)->value);
    while(a->p<a->file.s) {
        op=*par_this(a);
        if(!((op.type==STAR)||(op.type==SLASH))) {
            if(a->debug) printf("ne to! vmesto */ naydeno %s \r\n", op.value);
            break;
        }
        a->p++;
        AstNode *r=par_par_primary(a);
        l=(AstNode *)ast_create_binary(op.type, l, r);
    }
    return l;
}

AstNode *par_par_expr(Parser *a) {
    Token op, *t=par_this(a);
    AstNode *arg;
    if(a->debug) puts("par expr start");
    switch(t->type) {
        case LCALL: {
            a->p++;
            AstStmtCall *asc;
            if(a->debug) puts("lcall");
            Token *as=par_this(a);
            a->p++;
            expect(a, LBRACE, "\'(\'");
            if(a->debug) printf("this token: \'%s\' \r\n", par_this(a)->value);
            arg=par_par_expr(a);
            expect(a, RBRACE, "\')\'");
            asc=ast_create_call(as->value, arg);
            return (AstNode *)asc;
        }
    }
    AstNode *l=par_par_term(a);
    if(a->debug) puts("after calling term (in expr)");
    while(a->p<a->file.s) {
        op=*par_this(a);
        if(a->debug) printf("par_expr: op=\'%s\' \r\n", op.value);
        if(!((op.type==PLUS)||(op.type==MINUS))) {
            if(a->debug) printf("ne to! vmesto +- naydeno %s \r\n", op.value);
            break;
        }
        a->p++;
        AstNode *r=par_par_term(a);
        l=(AstNode *)ast_create_binary(op.type, l, r);
    }
    return l;
}

void par_optim(Parser *a) {
    a->root=(AstStmtBlock *)fold((AstNode *)a->root);
}
