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

#include "parser.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>

static Scope nullscope;

uint8_t par_stt(ds s) {
    if(SEQU(s, "Int")) return INT;
    if(SEQU(s, "Xshort")) return XSHORT;
    if(SEQU(s, "Char")) return CHAR;
    if(SEQU(s, "String")) return STR;
    if(SEQU(s, "Long")) return LONG;
    return 0;
}

char *par_tts(uint8_t t) {
    switch(t) {
        case INT: return "Int";
        case LONG: return "Long";
        case CHAR: return "Char";
        case STR: return "String";
    }
    return "Unknown";
}

void par_parFile(Parser *a) {
    AstNode *an;
    a->p=0;
    while(a->p<a->file.s) {
        an=par_par_expr(a);
        if(an) ast_add((AstNode *)a->root, an);
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
Token *par_post(Parser *a) {
    return (Token *)cv_eptr(&a->file, a->p++);
}

void par_init(Parser *n, const ds fni, const ds fno, int deb) {
    FILE *a=fopen(fni, "rb");
    memset(n, 0, sizeof(Parser));
    n->fn=strdup(fno);
    cv_init(&n->code, 32, sizeof(Vm));
    n->hp=0;
    n->heap=malloc(n->hs=1024);
    n->root=ast_create_block();
    cv_init(&n->libs, 4, sizeof(Lib));
    scos_init(&n->scopes);
    scos_es(&n->scopes);
    cv_init(&n->file, 16, sizeof(Token));
    n->debug=deb;
    ds x;
    while(!feof(a)) {
        ds_gl(&x, a);
        lexer(n, x);
    }
    fclose(a);
}

void par_render(Parser *a, uint8_t op, int64_t value) {
    Vm vm;
    vm.opcode=op, vm.value=value;
    cv_push(&a->code, &vm);
}

ds par_mangle(Func *f) {
    ds arg, ret=strdup(f->name);
    size_t i;
    for(i=0; i<f->args.s; i++) {
        switch(*(char *)cv_eptr(&f->args, i)) {
            case INT: arg="_i"; break;
            case STR: arg="_s"; break;
        }
        ds_cat(&ret, arg, NULL);
    }
    return ret;
}

void par_free(Parser *z) {
    cv final;
    size_t i, j;
    puts("\r\npar free \r\n=====");
    cv_init(&final, 32, 1);
    for(i=0; i<z->libs.s; i++) {
        Lib *lib=cv_eptr(&z->libs, i);
        for(int j=0; j<lib->funcs.s; j++) {
            Func *func=cv_eptr(&lib->funcs, j);
            printf("%s \r\n", par_mangle(func));
        }
    }
    if(z->debug) printf("par_free: fn = '%s'\n", z->fn);
    FILE *fp=fopen(z->fn, "wb");
    if(z->debug) printf("par_free: fp = %p\n", (void*)fp);
    uint32_t magic=0x05020200;
    uint64_t size=0; // 4 + 5 + 5 = 14 !!!!!
    fwrite(&magic, 1, sizeof(uint32_t), fp); ///// 0 2 1 5     9 0 0 0 0 10 0 0 0 0
    if(z->debug) puts("writing heap...");
    size=z->hp;
    fwrite(&size, 1, sz(size), fp);
    fwrite(z->heap, 1, size, fp);

    for(i=0; i<z->code.s; i++) {
        Vm *vm=cv_eptr(&z->code, i);
        cv_push(&final, &vm->opcode);
        size=selsz(vm->value);
        cv_push(&final, &size);
        for(j=0; j<size; j++) cv_push(&final, ((char *)&vm->value)+j);
    }

    size=final.s;
    fwrite(&size, 1, sz(size), fp);
    fwrite(final.d, 1, size, fp);
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

void lib_add(Parser *a, ds n) {
    Lib b;
    cv_init(&b.funcs, 16, sizeof(Func));
    b.name=strdup(n);
    cv_push(&a->libs, &b);
}

void func_add(Lib *a, Func z) {
    Func *copy=malloc(sizeof(Func));
    copy->name=strdup(z.name);
    cv_copy(&copy->args, &z.args);
    cv_push(&a->funcs, copy);
}

int func_find(Lib *a, Func z) {
    Func *t;
    size_t i;
    for(i=0; i<a->funcs.s; i++) {
        t=cv_eptr(&a->funcs, i);
        if(!strcmp(t->name, z.name)&&cv_equ(&t->args, &z.args)) return 1;
    }
    return 0;
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
        case ID: {
            variable var;
            scos_getv(&a->scopes, t.value, &var);
            if(!var.name) error("Undeclared variable \'%s\'", t.value);
            return (AstNode *)ast_create_variable(t.value);
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
    a->p++;
    switch(t->type) {
        case NATIVE: {
            Token *n=par_post(a);
            Func z;
            uint8_t c;
            printf("library \'%s\', functions: \r\n", n->value);
            lib_add(a, n->value);
            expect(a, LPAREN, "\'{\'");
            do {
                n=par_post(a);
                if(n->type!=ID) error("Expected function name!");
                printf("%s ( ", n->value);
                expect(a, LBRACE, "\'(\'");
                cv_init(&z.args, 4, 1);
                z.name=strdup(n->value);
                do {
                    n=par_post(a);
                    c=par_stt(n->value);
                    cv_push(&z.args, &c);
                    printf("%s ", n->value);
                    if(par_this(a)->type==COMMA) a->p++;
                } while(par_this(a)->type!=RBRACE);
                expect(a, RBRACE, "\')\'");
                printf(") \r\n");
                func_add((Lib *)cv_back(&a->libs), z);
                if(par_this(a)->type==COMMA) a->p++;
            } while (par_this(a)->type!=RPAREN);
            expect(a, RPAREN, "\'}\'");
            return NULL;
        }
        case LCALL: {
            cv q;
            Token *m=par_post(a);
            AstNode *n;
            cv_init(&q, 4, sizeof(AstNode *));
            if(a->debug) puts("lcall");
            expect(a, LBRACE, "\'(\'");
            do {
                n=par_par_comp(a);
                cv_push(&q, &n);
            } while(par_this(a)->type==COMMA&&a->p++);
            expect(a, RBRACE, "\')\'");
            return (AstNode *)ast_create_call(m->value, q);
        }
        case VAR: {
            AstStmtVarDecl *svd;
            Token *n, *w=par_post(a);
            AstNode *decl;
            AstNode *res=NULL;
            do {
                n=par_post(a);
                if(a->debug) printf("var; type=\'%s\', name=\'%s\' \r\n", w->value, n->value);
                scos_addv(&a->scopes, n->value, par_stt(w->value));
                decl=(AstNode*)ast_create_vardecl(n->value, par_stt(w->value));
                if(!res) {
                    res=decl;
                } else {
                    res=(AstNode*)ast_create_comma(res, decl);
                }
            } while(par_this(a)->type==COMMA&&a->p++);
            return res;
        }
        case IF: {
            AstStmtBlock *body;
            AstNode *cond;
            expect(a, LBRACE, "(");
            cond=par_par_comp(a);
            expect(a, RBRACE, ")");
            body=par_parBlock(a);
            return (AstNode *)ast_create_if(cond, body, NULL);
        }
        case WHILE: {
            AstStmtBlock *body;
            AstNode *cond;
            expect(a, LBRACE, "(");
            cond=par_par_comp(a);
            expect(a, RBRACE, ")");
            body=par_parBlock(a);
            return (AstNode *)ast_create_while(cond, body);
        }
        case ID: {
            variable var;
            Token *w=par_this(a);
            scos_getv(&a->scopes, t->value, &var);
            if(!var.name) error("Undeclared variable: %s", t->value);
            if(w->type==ASSIGN) {
                a->p++;
                return ast_create_assign(t->value, par_par_comp(a));
            }
        }
        default: a->p--;
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
    l=par_par_comma(a, l);
    return l;
}

AstNode *par_par_comma(Parser *a, AstNode *l) {
    while(a->p<a->file.s&&par_this(a)->type==COMMA) {
        a->p++;
        AstNode *r=par_par_comp(a);
        l=(AstNode*)ast_create_comma(l, r);
    }
    return l;
}

AstNode *par_par_comp(Parser *a) {
    AstNode *left=par_par_expr(a), *right;
    Token *op, *peek;
    while(a->p<a->file.s) {
        peek=par_this(a);
        switch(peek->type) {
            case EQ:
            case NEQ:
            case LT:
            case BT:
            case LE:
            case BE:
                break;
            default: return left;
        }
        op=par_post(a);
        right=par_par_comp(a);
        left=(AstNode *)ast_create_binary(op->type, left, right);
    }
    return left;
}

void par_optim(Parser *a) {
    a->root=(AstStmtBlock *)fold((AstNode *)a->root);
}
AstStmtBlock *par_parBlock(Parser *a) {
    AstStmtBlock *block=ast_create_block();
    AstNode *an;
    while(a->p<a->file.s&&par_this(a)->type!=RPAREN) {
        an=par_par_expr(a);
        if(an) ast_add(block, an);
    }
    return block;
}
