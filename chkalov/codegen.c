#include "codegen.h"

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
        case AST_STMT_VAR_DECL: {
            AstStmtVarDecl *svd=(AstStmtVarDecl *)node;
            par_render(a, PUSH, svd->var_type, 0);
            par_render(a, STORE, IDX|selszu(scos_ggi(&a->scopes)), scos_ggi(&a->scopes));
            break;
        }
    }
}
