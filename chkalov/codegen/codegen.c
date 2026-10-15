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
            size_t i, pos;
            uint8_t t;
            AstStmtCall *call=(AstStmtCall *)node;
            Lib *lib;
            Func key;
            cv types;
            cv_init(&types, 4, 1);
            AstNode **args=(AstNode **)call->args.d;
            for(i=0; i<call->args.s; i++) {
                t=type_expr(args[i], a);
                cv_push(&types, &t);
            }
            for(i=0; i<a->libs.s; i++) {
                size_t j;
                lib=cv_eptr(&a->libs, i);
                key.name=call->name;
                key.args=types;

                if(func_find(lib, key)) {
                    ds final=strdup(lib->name), mangled=par_mangle(&key);
                    ds_cat(&final, ".", mangled, NULL);
                    for(j=0; j<call->args.s; j++) codegen(args[j], a);
                    pos=par_heapIns(a, final);
                    par_render(a, PUSH, call->args.s);
                    par_render(a, CALL, pos);
                    free(final);
                    free(mangled);
                    cv_free(&types);
                    return;
                }
            }
            UserFunc *uf=user_func_find(&a->funcs, call->name);
            if(!uf) error("Undeclared function: \'%s\'", call->name);
            size_t j;
            for(j=0; j<call->args.s; j++) {
                codegen(args[j], a);
                par_render(a, ARG, 0);
            }

            par_render(a, INVOKE, uf->addr);
            return;
        }
        case AST_EXPR_LITERAL: {
            long long j;
            AstExprLiteral *lit=(AstExprLiteral *)node;
            switch(lit->lit_type) {
                case NUMBER: {
                    j=atoll(lit->value);
                    par_render(a, PUSH, j);
                    if(a->debug) puts("push long");
                    break;
                }
                case STRING: {
                    j=par_heapIns(a, lit->value);
                    par_render(a, S_PUSH, j);
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
                case PLUS: par_render(a, ADD, 0); break;
                case MINUS: par_render(a, SUB, 0); break;
                case STAR: par_render(a, MUL, 0); break;
                case SLASH: par_render(a, DIV, 0); break;
                case EQ: par_render(a, CMP_EQ, 0); break;
                case NEQ: par_render(a, CMP_NEQ, 0); break;
                case LT: par_render(a, CMP_LT, 0); break;
                case BT: par_render(a, CMP_BT, 0); break;
                case LE: par_render(a, CMP_LE, 0); break;
                case BE: par_render(a, CMP_BE, 0); break;
                default: error("Unknown binary operator: %d", bin->op);
            }
            break;
        }
        case AST_STMT_VAR_DECL: {
            AstStmtVarDecl *svd=(AstStmtVarDecl *)node;
            variable var;
            scos_getv(&a->scopes, svd->name, &var);
            par_render(a, PUSH, 0);
            par_render(a, STORE, var.id);
            break;
        }
        case AST_EXPR_COMMA: {
            AstExprComma *com=(AstExprComma *)node;
            codegen(com->left, a);
            codegen(com->right, a);
            break;
        }
        case AST_EXPR_VARIABLE: {
            AstExprVariable *var=(AstExprVariable *)node;
            variable vat;
            scos_getv(&a->scopes, var->name, &vat);
            par_render(a, LOAD, vat.id);
            break;
        }
        case AST_STMT_ASSIGN: {
            AstStmtAssign *ass=(AstStmtAssign *)node;
            variable vat;
            codegen(ass->value, a);
            par_render(a, DUP, 0);
            scos_getv(&a->scopes, ass->name, &vat);
            par_render(a, STORE, vat.id);
            break;
        }
        case AST_STMT_IF: {
            AstStmtIf *ifi=(AstStmtIf *)node;
            uintptr_t cs, tar;
            uint8_t e;
            codegen(ifi->cond, a);
            cs=a->code.s;
            par_render(a, JMP_IFN, 0);
            codegen(ifi->body, a);
            tar=a->code.s;
            ((Vm *)cv_eptr(&a->code, cs))->value=tar;
            break;
        }
        case AST_STMT_WHILE: {
            AstStmtWhile *whi=(AstStmtWhile *)node;
            uintptr_t ls=a->code.s, cs, tar;
            uint8_t e;
            codegen(whi->cond, a);
            cs=a->code.s;
            par_render(a, JMP_IFN, 0);
            printf("WHILE: before body: s=%zu\n", a->code.s);
            codegen(whi->body, a);
            printf("WHILE: after body: s=%zu\n", a->code.s);
            par_render(a, JUMP, ls);
            tar=a->code.s;
            printf("WHILE: after jump: s=%zu, tar=%zu\n", a->code.s, tar);
            ((Vm *)cv_eptr(&a->code, cs))->value=tar;
            printf("WHILE: ls=%zu, cs=%zu, tar=%zu\n", ls, cs, tar);
            break;
        }

        case AST_STMT_FUN: {
            AstStmtFun *fun=(AstStmtFun *)node;
            size_t addr=a->code.s;

            UserFunc *uf=user_func_find(&a->funcs, fun->name);
            if(uf) {
                uf->addr=addr;
            } else {
                user_func_add(&a->funcs, fun->name, addr, fun->ret_type);
            }
            printf("func: %s at %zx, body=%p, count=%zu\n", fun->name, addr, fun->body, fun->body->count);
            if(a->debug) printf("func: %s at %zx\n", fun->name, addr);

            if(!strcmp(fun->name, "main")) {
                a->entry_point=addr;
            }
            char buf[256];
            sprintf(buf, "E%llu%s(II)I", addr, fun->name);
            par_pool_add(a, buf);
            codegen(fun->body, a);
            break;
        }
        case AST_STMT_RET: {
            AstStmtRet *ret=(AstStmtRet *)node;
            codegen(ret->value, a);
            par_render(a, RET, 0);
            break;
        }
    }
}

uint8_t type_expr(AstNode *node, Parser *a) {
    int conv_table[6][6]={
    //             VOID CHAR SHORT INT LONG STR
    /* VOID */  {  0,   0,   0,    0,  0,   0},
    /* CHAR */  {  0,   1,   1,    1,  1,   0},
    /* SHORT */ {  0,   0,   1,    1,  1,   0},
    /* INT */   {  0,   0,   0,    1,  1,   0},
    /* LONG */  {  0,   0,   0,    0,  1,   0},
    /* STR  */  {  0,   0,   0,    0,  0,   1},
    };

    switch(node->type) {
        case AST_EXPR_LITERAL: {
            AstExprLiteral *lit=(AstExprLiteral *)node;
            return (lit->lit_type==NUMBER)?INT:(lit->lit_type==STRING)?STR:error("Error");
        }
        case AST_EXPR_VARIABLE: {
            AstExprVariable *var=(AstExprVariable *)node;
            variable v;
            scos_getv(&a->scopes, var->name, &v);
            return v.type;
        }
        case AST_STMT_CALL: {
            size_t i, pos;
            uint8_t t;
            AstStmtCall *call=(AstStmtCall *)node;
            Lib *lib;
            Func key;
            cv types;
            cv_init(&types, 4, 1);
            AstNode **args=(AstNode **)call->args.d;
            for(i=0; i<call->args.s; i++) {
                t=type_expr(args[i], a);
                cv_push(&types, &t);
            }
            for(i=0; i<a->libs.s; i++) {
                size_t j;
                lib=cv_eptr(&a->libs, i);
                key.name=call->name;
                key.args=types;

                if(func_find(lib, key)) {
                    ds final=strdup(lib->name), mangled=par_mangle(&key);
                    ds_cat(&final, ".", mangled, NULL);
                    for(j=0; j<call->args.s; j++) codegen(args[j], a);
                    pos=par_heapIns(a, final);
                    par_render(a, PUSH, call->args.s);
                    par_render(a, CALL, pos);
                    free(final);
                    free(mangled);
                    cv_free(&types);
                    return;
                }
            }
            UserFunc *uf=user_func_find(&a->funcs, call->name);
            if(!uf) error("Undeclared function: '%s'!", call->name);
            return INT;
        }
        case AST_BINARY: {
            AstBinary *bin=(AstBinary *)node;
            uint8_t left=type_expr(bin->left, a);
            uint8_t right=type_expr(bin->right, a);
            if(conv_table[left][right]) return MAX(left, right);
        } // fall-through
        default: return 0;
    }
}
