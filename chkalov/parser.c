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
        lexer(n, x);
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
        case LCALL: {
            AstStmtCall *asc;
            if(a->debug) puts("lcall");
            Token *as=par_post(a);
            expect(a, LBRACE, "\'(\'");
            if(a->debug) printf("this token: \'%s\' \r\n", par_this(a)->value);
            arg=par_par_comp(a);
            if(a->debug) printf("After par_par_comp: p=%zu, token='%s'\n", a->p, par_this(a)->value);
            expect(a, RBRACE, "\')\'");
            asc=ast_create_call(as->value, arg);
            return (AstNode *)asc;
        }
        case VAR: {
            AstStmtVarDecl *svd;
            Token *n, *w=par_post(a);
            AstNode *decl;
            AstNode *res=NULL;
            do {
                n=par_post(a);  // 'a', 'b', 'c'
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
