#include "sema_check.h"

void sem_analyze(AstNode *node, Parser *a) {
    if(!node||!a) return;
    switch(node->type) {
        case AST_STMT_BLOCK: {
            AstStmtBlock *block=(AstStmtBlock *)node;
            size_t i;
            for(i=0; i<block->count; i++) {
                sem_analyze(block->stats[i], a);
            }
            break;
        }

        case AST_STMT_CALL: {
            AstStmtCall *call=(AstStmtCall *)node;
            size_t i;
            for(i=0; i<call->args.s; i++) sem_analyze(*(AstNode **)cv_eptr(&call->args, i), a);
            break;
        }

        case AST_BINARY: {
            AstBinary *bin=(AstBinary *)node;
            sem_analyze(bin->left, a);
            sem_analyze(bin->right, a);
            Type left_type=type_expr(bin->left, a), right_type=type_expr(bin->right, a);
            if(right_type==T_VOID||left_type==T_VOID) error("Void in expression");
            if(!sem_binary_type(bin->op, left_type, right_type)) {
                error("Mismatch types: %s vs %s", par_tts(left_type), par_tts(right_type));
            }
        }

        case AST_STMT_ASSIGN: {
            break;
            AstStmtAssign *assign=(AstStmtAssign *)node;
            Variable *var;
            Type varType;
            sem_analyze(assign->value, a);
            var=scopes_getVar(&a->scopes, assign->name);
            varType=type_expr(assign->value, a);
            if(varType==T_VOID) error("Void in assignment");
            if(!sem_types_compatible(varType, var->type)) error("Cannot assign %s to %s", par_tts(varType), par_tts(var->type));
            break;
        }

        case AST_EXPR_COMMA: {
            AstExprComma *comma=(AstExprComma *)node;
            sem_analyze(comma->left, a);
            sem_analyze(comma->right, a);
            break;
        }

        case AST_STMT_IF: {
            AstStmtIf *iff=(AstStmtIf *)node;
            sem_analyze(iff->cond, a);
            sem_analyze(iff->body, a);
            sem_analyze(iff->els, a);
            break;
        }

        case AST_STMT_WHILE: {
            AstStmtWhile *whi=(AstStmtWhile *)node;
            sem_analyze(whi->cond, a);
            sem_analyze(whi->body, a);
            break;
        }

        case AST_STMT_FUN: {
            AstStmtFun *fun=(AstStmtFun *)node;
            sem_analyze((AstNode *)fun->body, a);
            break;
        }
        default: return;
    }
}

Type sem_binary_type(TokType op, Type left_type, Type right_type) {
    return binary_result[left_type][right_type];
}
/*
bool sem_types_compatible(Type left_type, Type right_type) {
    return !!comp_result[left_type][right_type];
}*/

void collect_funcs(AstNode *node, Parser *a) {
    if(!node) return;
    switch(node->type) {
        case AST_STMT_BLOCK: {
            AstStmtBlock *block=(AstStmtBlock *)node;
            for(size_t i=0; i<block->count; i++) {
                collect_funcs(block->stats[i], a);
            }
            break;
        }
        case AST_STMT_FUN: {
            AstStmtFun *fun=(AstStmtFun *)node;
            user_func_add(&a->funcs, fun->name, 0, fun->ret_type);
            collect_funcs((AstNode *)fun->body, a);
            break;
        }
        default: break;
    }
}
