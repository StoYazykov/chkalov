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

#include "ast.h"
#include "parser.h"
#include <stdlib.h>
#include <stdio.h>

AstStmtBlock* ast_create_block() {
    AstStmtBlock *prog=malloc(sizeof(AstStmtBlock));
    prog->base.type=AST_STMT_BLOCK;
    prog->stats=NULL;
    prog->count=0;
    return prog;
}

AstStmtCall* ast_create_call(char *n, AstNode *arg) {
    AstStmtCall *call=malloc(sizeof(AstStmtCall));
    call->base.type=AST_STMT_CALL;
    call->name=strdup(n);
    call->arg=arg;
    return call;
}

AstExprLiteral* ast_create_literal(TokenType t, char *v) {
    AstExprLiteral *lit=malloc(sizeof(AstExprLiteral));
    lit->base.type=AST_EXPR_LITERAL;
    lit->type=t;
    lit->value=strdup(v);
    return lit;
}

AstBinary *ast_create_binary(TokenType t, AstNode *l, AstNode *r) {
    AstBinary *bin=malloc(sizeof(AstBinary));
    bin->base.type=AST_BINARY;
    bin->op=t;
    bin->left=l;
    bin->right=r;
    return bin;
}

AstExprVariable *ast_create_variable(char *n) {
    AstExprVariable *var=malloc(sizeof(AstExprVariable));
    var->name=strdup(n);
    var->base.type=AST_EXPR_VARIABLE;
    return var;
}

AstStmtAssign *ast_create_assign(char *n, AstNode *v) {
    AstStmtAssign *ass=malloc(sizeof(AstStmtAssign));
    ass->name=strdup(n);
    ass->value=v;
    ass->base.type=AST_STMT_ASSIGN;
    return ass;
}

AstStmtVarDecl *ast_create_vardecl(char *n, uint8_t vt) {
    AstStmtVarDecl *svd=malloc(sizeof(AstStmtVarDecl));
    svd->name=strdup(n);
    svd->var_type=vt;
    svd->base.type=AST_STMT_VAR_DECL;
    return svd;
}

AstExprComma *ast_create_comma(AstNode *l, AstNode *r) {
    AstExprComma *com=malloc(sizeof(AstExprComma));
    com->base.type=AST_EXPR_COMMA;
    com->left=l;
    com->right=r;
    return com;
}

AstStmtIf *ast_create_if(AstNode *c, AstStmtBlock *b, AstStmtBlock *e) {
    AstStmtIf *ifi=malloc(sizeof(AstStmtIf));
    ifi->base.type=AST_STMT_IF;
    ifi->cond=c;
    ifi->body=b;
    ifi->els=e;
    return ifi;
}

AstStmtWhile *ast_create_while(AstNode *c, AstStmtBlock *b) {
    AstStmtWhile *whi=malloc(sizeof(AstStmtWhile));
    whi->base.type=AST_STMT_WHILE;
    whi->cond=c;
    whi->body=b;
    return whi;
}

void ast_free(AstNode *node) {
    if(!node) return;
    switch(node->type) {
        case AST_STMT_BLOCK: {
            AstStmtBlock *p=(AstStmtBlock*)node;
            for(size_t i=0; i<p->count; i++) ast_free(p->stats[i]);
            free(p->stats);
            free(p);
            break;
        }
        case AST_STMT_CALL: {
            AstStmtCall *c=(AstStmtCall *)node;
            free(c->name);
            ast_free(c->arg);
            free(c);
            break;
        }
        case AST_EXPR_LITERAL: {
            AstExprLiteral *l=(AstExprLiteral *)node;
            free(l->value);
            free(l);
            break;
        }
        case AST_BINARY: {
            AstBinary *b=(AstBinary *)node;
            ast_free(b->left);
            ast_free(b->right);
            free(b);
            break;
        }
        case AST_STMT_ASSIGN: {
            AstStmtAssign *a=(AstStmtAssign *)node;
            ast_free(a->value);
            free(a->name);
            break;
        }
        case AST_EXPR_VARIABLE: {
            AstExprVariable *v=(AstExprVariable *)node;
            free(v->name);
            break;
        }
        case AST_STMT_VAR_DECL: {
            AstStmtVarDecl *d=(AstStmtVarDecl *)node;
            free(d->name);
            break;
        }
        case AST_EXPR_COMMA: {
            AstExprComma *c=(AstExprComma *)node;
            ast_free(c->left);
            ast_free(c->right);
            free(c);
            break;
        }
        case AST_STMT_WHILE: {
            AstStmtWhile *w=(AstStmtWhile *)node;
            ast_free(w->cond);
            ast_free(w->body);
            free(w);
            break;
        }
    }
}

void ast_add(AstNode *r, AstNode *n) {
    switch(r->type) {
        case AST_STMT_BLOCK: {
            AstStmtBlock *y;
            y=(AstStmtBlock *)r;
            y->stats=realloc(y->stats, sizeof(AstNode *)*(y->count+1));
            y->stats[y->count++]=(AstNode *)n;
            break;
        }
    }
}

void ast_print(AstNode *node, int indent) {
    for(int i=0; i<indent; i++) printf("  ");
    switch(node->type) {
        case AST_STMT_BLOCK:
            puts("Program");
            AstStmtBlock *p=(AstStmtBlock *)node;
            for(size_t i=0; i<p->count; i++) ast_print(p->stats[i], indent+1);
            break;
        case AST_STMT_CALL: {
            AstStmtCall *c=(AstStmtCall *)node;
            printf("CALL: %s \r\n", c->name);
            ast_print(c->arg, indent+1);
            break;
        }
        case AST_EXPR_LITERAL: {
            AstExprLiteral *l=(AstExprLiteral *)node;
            printf("LITERAL: %s \r\n", l->value);
            break;
        }
        case AST_BINARY: {
            AstBinary *b=(AstBinary *)node;
            puts("BINARY");
            ast_print(b->left, indent+1);
            ast_print(b->right, indent+1);
            break;
        }
        case AST_STMT_ASSIGN: {
            AstStmtAssign *a=(AstStmtAssign *)node;
            puts("ASSIGN");
            printf("  NAME: %s \r\n", a->name);
            ast_print(a->value, indent+1);
            break;
        }
        case AST_EXPR_VARIABLE: {
            AstExprVariable *v=(AstExprVariable *)node;
            printf("VARIABLE \r\n  NAME: %s \r\n", v->name);
            break;
        }
        case AST_STMT_VAR_DECL: {
            AstStmtVarDecl *d=(AstStmtVarDecl *)node;
            printf("VARIABLE DECL \r\n    NAME: %s \r\n    TYPE: %s \r\n", d->name, par_tts(d->var_type));
            break;
        }
        case AST_EXPR_COMMA: {
            AstExprComma *c=(AstExprComma *)node;
            puts("COMMA");
            ast_print(c->left, indent+1);
            ast_print(c->right, indent+1);
            break;
        }
        case AST_STMT_IF: {
            AstStmtIf *stmt=(AstStmtIf *)node;
            printf("IF\n");
            printf("CONDITION:\n");
            ast_print(stmt->cond, indent+1);
            printf("BODY:\n");
            ast_print(stmt->body, indent+1);
            if(stmt->els) {
                printf("ELSE:\n");
                ast_print(stmt->els, indent+1);
            }
            break;
        }
        case AST_STMT_WHILE: {
            AstStmtWhile *stmt=(AstStmtWhile *)node;
            printf("WHILE\n");
            printf("CONDITION:\n");
            ast_print(stmt->cond, indent+1);
            printf("BODY:\n");
            ast_print(stmt->body, indent+1);
            break;
        }
    }
}

AstNode *fold(AstNode *node) {
    if(!node) return NULL;
    switch(node->type) {
        case AST_STMT_BLOCK: {
            AstStmtBlock *block=(AstStmtBlock *)node;
            for(size_t i=0; i<block->count; i++) {
                block->stats[i]=fold((AstNode *)block->stats[i]);
            }
            return (AstNode *)block;
        }
        case AST_STMT_CALL: {
            AstStmtCall *c=(AstStmtCall*)node;
            c->arg=fold(c->arg);
            return node;
        }
        case AST_BINARY: {
            AstBinary *b=(AstBinary *)node;
            AstExprLiteral *l, *r;
            if(!(l&&r)) return node;
            b->left=fold(b->left);
            b->right=fold(b->right);
            l=(AstExprLiteral *)b->left, r=(AstExprLiteral *)b->right;
            if(b->left->type!=AST_EXPR_LITERAL||b->right->type!=AST_EXPR_LITERAL) return node;
            int64_t x=atoll(l->value), y=atoll(r->value), e;
            switch(b->op) {
                case PLUS: e=x+y; break;
                case MINUS: e=x-y; break;
                case STAR: e=x*y; break;
                case SLASH: e=x/y; break;
                case EQ: e=x==y; break;
                case NEQ: e=x!=y; break;
                case LT: e=x<y; break;
                case BT: e=x>y; break;
                case LE: e=x<=y; break;
                case BE: e=x>=y; break;
                default: return node;
            }
            ast_free(b);
            char buf[32];
            sprintf(buf, "%lld", e);
            return (AstNode *)ast_create_literal(NUMBER, buf);
        }
        case AST_EXPR_COMMA: {
            AstExprComma *c=(AstExprComma *)node;
            c->left=fold(c->left);
            c->right=fold(c->right);
            return c;
        }
        case AST_STMT_ASSIGN: {
            AstStmtAssign *s=(AstStmtAssign *)node;
            s->value=fold(s->value);
            return s;
        }
        case AST_EXPR_VARIABLE:
        case AST_STMT_VAR_DECL:
        case AST_EXPR_LITERAL:
        case AST_STMT_IF:
        case AST_STMT_WHILE:
            return node;
    }
}
