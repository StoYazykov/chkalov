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
                case EQ: par_render(a, CMP_EQ, 0, 0); break;
                case NEQ: par_render(a, CMP_NEQ, 0, 0); break;
                case LT: par_render(a, CMP_LT, 0, 0); break;
                case BT: par_render(a, CMP_BT, 0, 0); break;
                case LE: par_render(a, CMP_LE, 0, 0); break;
                case BE: par_render(a, CMP_BE, 0, 0); break;
                default: error("Unknown binary operator: %d", bin->op);
            }
            break;
        }
        case AST_STMT_VAR_DECL: {
            AstStmtVarDecl *svd=(AstStmtVarDecl *)node;
            variable var;
            scos_getv(&a->scopes, svd->name, &var);
            par_render(a, PUSH, svd->var_type, 0);
            par_render(a, STORE, IDX|selszu(var.id), var.id);
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
            par_render(a, LOAD, IDX|selszu(vat.id), vat.id);
            break;
        }
        case AST_STMT_ASSIGN: {
            AstStmtAssign *ass=(AstStmtAssign *)node;
            variable vat;
            codegen(ass->value, a);
            par_render(a, DUP, 0, 0);
            scos_getv(&a->scopes, ass->name, &vat);
            par_render(a, STORE, IDX|selszu(vat.id), vat.id);
            break;
        }
        case AST_STMT_IF: {
            AstStmtIf *ifi=(AstStmtIf *)node;
            size_t cs=a->code.s, tar;
            break;
        }
    }
}
