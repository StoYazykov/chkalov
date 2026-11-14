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
            Type t;
            AstStmtCall *call=(AstStmtCall *)node;
            Lib *lib;
            Func key;
            cv types;
            cv_init(&types, 4, sz(Type));
            AstNode **args=(AstNode **)call->args.d;
            for(i=0; i<call->args.s; i++) {
                t=type_expr(args[i], a);
                cv_push(&types, &t);
            }
            puts("Codegen: findeing natives");
            for(i=0; i<a->libs.s; i++) {
                size_t j;
                printf("Iteration %d\r\n");
                lib=cv_eptr(&a->libs, i);
                key.name=call->name;
                key.args=types;

                if(func_find(lib, key)) {
                    ds final=strdup(lib->name), mangled=par_mangle(&key);
                    ds_cat(&final, ".", mangled, NULL);
                    for(j=0; j<call->args.s; j++) codegen(args[j], a);
                    pos=par_heapIns(a, final);
                    par_render(a, OP_IPUSH, call->args.s);
                    par_render(a, OP_CALL, pos);
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
                par_render(a, OP_ARG, 0);
            }
            par_render(a, OP_INVOKE, 0);
            InvokePatch pt;
            pt.pos=a->code.s-1;
            pt.name=strdup(call->name);
            cv_push(&a->invoke_patches, &pt);
            return;
        }
        case AST_EXPR_LITERAL: {
            AstExprLiteral *lit=(AstExprLiteral *)node;
            if(lit->lit_type==NUMBER) {
                switch(lit->suffix) {
                    case SUFFIX_NONE: puts("type_expr: Suffix None"); par_render(a, OP_IPUSH, atol(lit->value)); break;
                    case SUFFIX_LONG: puts("type_expr: Suffix Long"); par_render(a, OP_LPUSH, atol(lit->value)); break;
                    default: error("TYPE_EXPR SUFFIX ERROR"); break;
                }
            } else {
                size_t offset=par_heapIns(a, lit->value);
                par_render(a, OP_APUSH, offset);
            }
            return;
        }
        case AST_BINARY: {
            AstBinary *bin=(AstBinary *)node;
            codegen(bin->left, a);
            codegen(bin->right, a);
            switch(bin->op) {
                case PLUS: par_render(a, OP_IADD, 0); break;
                case MINUS: par_render(a, OP_ISUB, 0); break;
                case STAR: par_render(a, OP_IMUL, 0); break;
                case SLASH: par_render(a, OP_IDIV, 0); break;
                case EQ: par_render(a, OP_IEQ, 0); break;
                case NEQ: par_render(a, OP_INEQ, 0); break;
                case LT: par_render(a, OP_ILT, 0); break;
                case BT: par_render(a, OP_IBT, 0); break;
                case LE: par_render(a, OP_ILE, 0); break;
                case BE: par_render(a, OP_IBE, 0); break;
                default: error("Unknown binary operator: %d", bin->op);
            }
            break;
        }
        case AST_STMT_VAR_DECL: {
            AstStmtVarDecl *svd=(AstStmtVarDecl *)node;
            par_render(a, OP_IPUSH, 0);
            par_render(a, OP_ISTORE, svd->id-a->current_base);
            break;
        }
        case AST_EXPR_COMMA: {
            AstExprComma *com=(AstExprComma *)node;
            codegen(com->left, a);
            codegen(com->right, a);
            break;
        }
        case AST_EXPR_VARIABLE: {
            AstExprVariable *var_expr=(AstExprVariable *)node;
            printf("LOAD %s id=%zu base=%zu v=%zu\n", var_expr->name, var_expr->id, a->current_base, var_expr->id-a->current_base);
            par_render(a, OP_ILOAD, var_expr->id-a->current_base);
            break;
        }
        case AST_STMT_ASSIGN: {
            AstStmtAssign *ass=(AstStmtAssign *)node;
            Variable *var;
            codegen(ass->value, a);
            par_render(a, OP_DUP, 0);
            var=scopes_getVar(&a->scopes, ass->name);
            par_render(a, OP_ISTORE, ass->id-a->current_base);
            break;
        }
        case AST_STMT_IF: {
            AstStmtIf *ifi=(AstStmtIf *)node;
            uintptr_t cs, tar;
            Type e;
            codegen(ifi->cond, a);
            cs=a->code.s;
            par_render(a, OP_JMP_IFN, 0);
            codegen(ifi->body, a);
            tar=a->code.s;
            ((Vm *)cv_eptr(&a->code, cs))->value=tar;
            break;
        }
        case AST_STMT_WHILE: {
            AstStmtWhile *whi=(AstStmtWhile *)node;
            uintptr_t ls=a->code.s, cs, tar;
            Type e;
            codegen(whi->cond, a);
            cs=a->code.s;
            par_render(a, OP_JMP_IFN, 0);
            printf("WHILE: before body: s=%zu\n", a->code.s);
            codegen(whi->body, a);
            printf("WHILE: after body: s=%zu\n", a->code.s);
            par_render(a, OP_JUMP, ls);
            tar=a->code.s;
            printf("WHILE: after jump: s=%zu, tar=%zu\n", a->code.s, tar);
            ((Vm *)cv_eptr(&a->code, cs))->value=tar;
            printf("WHILE: ls=%zu, cs=%zu,  tar=%zu\n", ls, cs, tar);
            break;
        }

        case AST_STMT_FUN: {
            AstStmtFun *fun=(AstStmtFun *)node;
            size_t addr=a->code.s, i;

            UserFunc *uf=user_func_find(&a->funcs, fun->name);
            if(uf) {
                uf->addr=addr;
            } else {
                user_func_add(&a->funcs, fun->name, addr, fun->ret_type);
            }
            printf("func: %s at %zx, body=%p, count=%zu\n", fun->name, addr, fun->body, fun->body->count);
            printf("fun %s base_id=%zu\n", fun->name, fun->base_id);
            if(a->debug) printf("func: %s at %zx\n", fun->name, addr);

            if(!strcmp(fun->name, "main")) {
                a->entry_point=addr;
            }
            char buf[256];
            sprintf(buf, "E(%llu)%s(", addr, fun->name);
            Type arg_type;
            for(i=0; i<fun->args.s; i++) {
                arg_type=*(Type *)cv_eptr(&fun->args, i);
                sprintf(buf+strlen(buf), "%c", par_ttc(arg_type));
            }
            sprintf(buf+strlen(buf), ");%c", par_ttc(fun->ret_type));
            par_pool_add(a, buf);
            size_t old_base=a->current_base;
            a->current_base=fun->base_id;
            codegen(fun->body, a);
            par_render(a, OP_IPUSH, 0);
            par_render(a, OP_RET, 0);
            a->current_base=old_base;
            break;
        }
        case AST_STMT_RET: {
            AstStmtRet *ret=(AstStmtRet *)node;
            codegen(ret->value, a);
            par_render(a, OP_RET, 0);
            break;
        }
    }
}

Type type_expr(AstNode *node, Parser *a) {
    switch(node->type) {
        case AST_EXPR_LITERAL: {
            AstExprLiteral *lit=(AstExprLiteral *)node;
            if(lit->lit_type==NUMBER) {
                switch(lit->suffix) {
                    case SUFFIX_NONE: puts("type_expr: Suffix None"); return T_INT;
                    case SUFFIX_LONG: puts("type_expr: Suffix Long"); return T_LONG;
                    default: error("TYPE_EXPR SUFFIX ERROR");
                }
            } else return T_STRING;
        }
        case AST_EXPR_VARIABLE: {
            AstExprVariable *var_expr=(AstExprVariable *)node;
            return var_expr->type;
        }
       case AST_STMT_CALL: {
            size_t i, pos;
            Type t;
            AstStmtCall *call=(AstStmtCall *)node;
            Lib *lib;
            Func key;
            cv types;
            cv_init(&types, 4, sz(Type));
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
                    //ds final=strdup(lib->name), mangled=par_mangle(&key);
                    //ds_cat(&final, ".", mangled, NULL);
                    //for(j=0; j<call->args.s; j++) codegen(args[j], a);
                    //pos=par_heapIns(a, final);
                    //par_render(a, PUSH, call->args.s);
                    //par_render(a, CALL, pos);
                    //free(final);
                    //free(mangled);
                    cv_free(&types);
                    return;
                }
            }
            UserFunc *uf=user_func_find(&a->funcs, call->name);
            if(!uf) error("Codegen: Undeclared function: \'%s\'!", call->name);
            return uf->ret_type;
        }
        case AST_BINARY: {
            AstBinary *bin=(AstBinary *)node;
            Type left=type_expr(bin->left, a), right=type_expr(bin->right, a);
            return left;
        } // fall-through
        default: return 0;
    }
}
