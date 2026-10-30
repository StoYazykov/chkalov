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
            uint8_t left_type=type_expr(bin->left, a), right_type=type_expr(bin->right, a);
            if(right_type==VOID||left_type==VOID) error("Void in expression");
            if(!sem_binary_type(bin->op, left_type, right_type)) {
                error("Mismatch types: %s vs %s", par_tts(left_type), par_tts(right_type));
            }
        }

        case AST_STMT_ASSIGN: {
            AstStmtAssign *assign=(AstStmtAssign *)node;
            Variable *var;
            uint8_t varType;
            sem_analyze(assign->value, a);
            var=scopes_getVar(&a->scopes, assign->name);
            varType=type_expr(assign->value, a);
            if(varType==VOID) error("Void in assignment");
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

uint8_t sem_binary_type(TokType op, uint8_t lt, uint8_t rt) {
    uint8_t arith_result[8][8]={
    //           VOID  CHAR  SHORT INT   LONG  UINT  ULONG STR
    /* VOID  */ {VOID, VOID, VOID, VOID, VOID, VOID, VOID, VOID},
    /* CHAR  */ {VOID, CHAR, SHORT,INT,  LONG, INT,  LONG, VOID},
    /* SHORT */ {VOID, SHORT,SHORT,INT,  LONG, INT,  LONG, VOID},
    /* INT   */ {VOID, INT,  INT,  INT,  LONG, UINT, ULONG,VOID},
    /* LONG  */ {VOID, LONG, LONG, LONG, LONG, ULONG,ULONG,VOID},
    /* UINT  */ {VOID, INT,  INT,  UINT, ULONG,UINT, ULONG,VOID},
    /* ULONG */ {VOID, LONG, LONG, ULONG,ULONG,ULONG,ULONG,VOID},
    /* STR   */ {VOID, VOID, VOID, VOID, VOID, VOID, VOID, STR},
    };

    switch(op) {
        case PLUS: case MINUS: case STAR: case SLASH:
        case EQ: case NEQ: case LT: case BT: case LE: case BE: return arith_result[lt][rt];
    }
}

bool sem_types_compatible(uint8_t lt, uint8_t rt) {
    int comp_result[8][8]={
    //           VOID  CHAR  SHORT INT   LONG  UINT  ULONG STR
    /* VOID  */ {0,    0,    0,    0,    0,    0,    0,    0},
    /* CHAR  */ {0,    1,    1,    1,    1,    1,    1,    0},
    /* SHORT */ {0,    0,    1,    1,    1,    1,    1,    0},
    /* INT   */ {0,    0,    0,    1,    1,    1,    1,    0},
    /* LONG  */ {0,    0,    0,    0,    1,    0,    1,    0},
    /* UINT  */ {0,    0,    0,    1,    1,    1,    1,    0},
    /* ULONG */ {0,    0,    0,    0,    1,    0,    1,    0},
    /* STR   */ {0,    0,    0,    0,    0,    0,    0,    1}
    };
    return !!comp_result[lt][rt];
}

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
