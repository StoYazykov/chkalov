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

void expect(Parser *a, TokType t, char *s) {
    if(par_this(a)->type==t) a->p++;
    else {
        if(par_this(a)->value) {
            error("Expected %s, detected: %s", s, par_this(a)->value);
        } else {
            error("Expected %s, but end of file", s);
        }
    }
}

void skip(Parser *a, TokType t) {
    if(par_this(a)->type==t) a->p++;
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
    cv_init(&n->funcs, 8, sizeof(UserFunc));
    cv_init(&n->libs, 4, sizeof(Lib));
    cv_init(&n->file, 16, sizeof(Token));
    cv_init(&n->pool, 8, sizeof(char *));
    cv_init(&n->invoke_patches, 8, sizeof(InvokePatch));

    n->heap=malloc(n->hs=1024);
    par_heapIns(n, "\0");
    n->root=ast_create_block();

    scopes_init(&n->scopes);
    scopes_enterScope(&n->scopes);

    n->debug=deb;
    ds x;

    while(!feof(a)) {
        ds_gl(&x, a);
        lexer(n, x);
    }
    fclose(a);
}

size_t par_pool_add(Parser *a, char *str) {
    char *dup=strdup(str);
    cv_push(&a->pool, &dup);
    return a->pool.s-1;
}

void par_render(Parser *a, uint8_t op, int64_t value) {
    Vm vm;
    vm.opcode=op, vm.value=value;
    cv_push(&a->code, &vm);
}

ds par_mangle(Func *f) {
    ds ret=strdup(f->name), arg;
    Type type;
    ds_cat(&ret, "_", NULL);
    size_t i;
    for(i=0; i<f->args.s; i++) {
        type=*(Type *)cv_eptr(&f->args, i);
        switch(type) {
            case T_CHAR:    arg="C"; break;
            case T_SHORT:   arg="X"; break;
            case T_INT:     arg="I"; break;
            case T_LONG:    arg="L"; break;
            case T_STRING:  arg="S"; break;
            case T_UCHAR:   arg="c"; break;
            case T_USHORT:  arg="x"; break;
            case T_UINT:    arg="i"; break;
            case T_ULONG:   arg="l"; break;
            default:        arg=malloc(32);sprintf(arg, "INCORRECT: type %d\r\n", type);
        }
        ds_cat(&ret, arg, NULL);
    }
    return ret;
}

void par_free(Parser *z) {
    cv final;
    size_t i, j;
    uint8_t size;
    puts("\r\npar free \r\n=====");
    cv_init(&final, 32, 1);
    for(i=0; i<z->libs.s; i++) {
        Lib *lib=cv_eptr(&z->libs, i);
        for(int j=0; j<lib->funcs.s; j++) {
            Func *func=cv_eptr(&lib->funcs, j);
            printf("native: %s \r\n", par_mangle(func));
        }
    }
    if(z->debug) printf("par_free: fn = '%s'\n", z->fn);
    FILE *fp=fopen(z->fn, "wb");
    char **p;
    if(z->debug) printf("par_free: fp = %p\n", (void*)fp);
    Header hdr;
    hdr.magic=0x05020200;

    for(i=0; i<z->code.s; i++) {
        Vm *vm=cv_eptr(&z->code, i);
        cv_push(&final, &vm->opcode);
        size=selsz(vm->value);
        cv_push(&final, &size);
        for(j=0; j<size; j++) cv_push(&final, ((char *)&vm->value)+j);
    }

    for(i=0; i<z->invoke_patches.s; i++) {
        InvokePatch *pt=cv_eptr(&z->invoke_patches, i);
        free(pt->name);
    }
    cv_free(&z->invoke_patches);

    hdr.code_size=final.s;
    hdr.heap_size=z->hp;
    hdr.entry_point=z->entry_point;
    hdr.pool_size=z->pool.s;

    fwrite(&hdr, 1, sz(Header), fp);

    fwrite(z->heap, 1, z->hp, fp);

    char **end=((char **)z->pool.d)+hdr.pool_size;
    uint32_t len;

    for(p=(char **)z->pool.d; p<end; p++) {
        len=strlen(*p)+1;
        fwrite(&len, 1, sz(uint32_t), fp);
        fwrite(*p, 1, len, fp);
        printf("POOL WRITED: \'%s\'\r\n", *p);
    }

    fwrite(final.d, 1, final.s, fp);

    fclose(fp);
    free(z->heap);
    cv_free(&z->pool);
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

void par_optim(Parser *a) {
    a->root=(AstStmtBlock *)fold((AstNode *)a->root);
}

AstStmtBlock *par_parBlock(Parser *a) {
    AstStmtBlock *block=ast_create_block();
    AstNode *an;
    expect(a, LPAREN, "{");
    while(a->p<a->file.s&&par_this(a)->type!=RPAREN) {
        an=par_par_expr(a);
        if(an) ast_add(block, an);
    }
    if(par_this(a)->type==RPAREN) a->p++;
    return block;
}

void par_split_commas(AstNode *n, cv *q) {
    if(n->type==AST_EXPR_COMMA) {
        AstExprComma *com=(AstExprComma *)n;
        par_split_commas(com->left, q);
        par_split_commas(com->right, q);
        free(com);
    } else {
        cv_push(q, &n);
    }
}
