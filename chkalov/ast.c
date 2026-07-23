#include "ast.h"
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
        case AST_STMT_CALL: {
            AstStmtCall *y;
            y=(AstStmtCall *)r;
            y->arg=n;
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
        case AST_EXPR_LITERAL: {
            return node;
        }
        case AST_BINARY: {
            AstBinary *b=(AstBinary *)node;
            AstExprLiteral *l, *r;
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
            }
            ast_free(b);
            char buf[32];
            sprintf(buf, "%lld", e);
            return (AstNode *)ast_create_literal(NUMBER, buf);
        }
    }
}
