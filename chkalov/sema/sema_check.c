#include "sema_check.h"

void sem_analyze(AstNode *node, Parser *a) {
    if(!node||!a) return;
    switch(node->type) {
        case AST_STMT_BLOCK: {
            AstStmtBlock *block=(AstStmtBlock *)node;
            size_t i;
            for(i=0; i<block->count; i++) sem_analyze(block->stats[i], a);
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
            Type left_type, right_type;
            sem_analyze(bin->left, a);
            sem_analyze(bin->right, a);
            left_type=type_expr(bin->left, a);
            right_type=type_expr(bin->right, a);
            if(left_type==T_INCORRECT||right_type==T_INCORRECT) error("Internal: incorrect type");
            if(left_type==T_VOID||right_type==T_VOID) error("Void in expression");
            if(sem_binary_type(bin->op, left_type, right_type)==T_INCORRECT)
                error("Mismatch types: %s vs %s", par_tts(left_type), par_tts(right_type));
            break;
        }
        case AST_STMT_ASSIGN: {
            AstStmtAssign *assign=(AstStmtAssign *)node;
            Variable *var;
            Type varType;
            scopes_enterScope(&a->scopes);
            sem_analyze(assign->value, a);
            printf("assign->name = \'%s\', calling scopes_getVar...\n", assign->name);
            var=scopes_getVar(&a->scopes, assign->name);
            if(!var) {
                Scope *scope;
                Variable *var2;
                for(scope=(Scope *)a->scopes.scopes.d; scope<((Scope *)a->scopes.scopes.d)+a->scopes.scopes.s; scope++) {
                    for(var2=(Variable *)scope->vars.d; var2<((Variable *)scope->vars.d)+scope->vars.s; var2++) {
                        printf("VAR: \'%s\': %s", var2->name, par_tts(var2->type));
                    }
                }
                error("Undeclared variable %s in assign", assign->name);
            }
            varType=type_expr(assign->value, a);
            if(varType==T_INCORRECT) error("Internal: incorrect type");
            if(varType==T_VOID) error("Void in assignment");
            if(!sem_types_compatible(varType, var->type))
                error("Cannot assign %s to %s", par_tts(varType), par_tts(var->type));
            break;
            scopes_exitScope(&a->scopes);
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
        default: break;
    }
}

void collect_funcs(AstNode *node, Parser *a) {
    if(!node) return;
    switch(node->type) {
        case AST_STMT_BLOCK: {
            AstStmtBlock *block=(AstStmtBlock *)node;
            for(size_t i=0; i<block->count; i++) collect_funcs(block->stats[i], a);
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

Type sem_binary_type(TokType op, Type left_type, Type right_type) {
    if(left_type<0||left_type>=T_COUNT) return T_INCORRECT;
    if(right_type<0||right_type>=T_COUNT) return T_INCORRECT;

    size_t idx;
    switch(op) {
        case PLUS:  idx=0; break;
        case MINUS: idx=1; break;
        case STAR:  idx=2; break;
        case SLASH: idx=3; break;
        case EQ:
        case NEQ:
        case LT:
        case BT:
        case LE:
        case BE:    idx=4; break;
        default:    return T_INCORRECT;
    }
    return binary_result[idx][left_type][right_type];
}

bool sem_types_compatible(Type left_type, Type right_type) {
    if(left_type<0||left_type>=T_COUNT) return false;
    if(right_type<0||right_type>=T_COUNT) return false;
    return types_compatible[left_type][right_type];
}
