#include "expressions_parser.h"

AstNode *par_par_primary(Parser *a) {
    Token t=*par_this(a);
    AstNode *arg;
    if(a->debug) puts("par primary start");
    a->p++;
    switch(t.type) {
        case NUMBER: {
            return (AstNode *)ast_create_literal(NUMBER, t.value, t.suffix);
        }

        case LBRACE: {
            AstNode *p;
            p=par_par_comp(a);
            expect(a, RBRACE, "')'");
            return p;
        }
        case ID: {
            Variable *var;
            var=scopes_getVar(&a->scopes, t.value);
            if(!var) error("Primary: Undeclared variable \'%s\'", t.value);
            return (AstNode *)ast_create_variable(t.value, var->id, var->type);
        }
        case STRING: return(AstNode *)ast_create_literal(STRING, t.value, SUFFIX_NONE);
    }
    return NULL;
}

AstNode *par_par_term(Parser *a) {
    Token op;
    if(a->debug) puts("par term start");
    AstNode *l=par_par_primary(a);
    if(a->debug&&a->p<a->file.s) printf("after calling primary (in term): p=%llu token='%s'\n", a->p, par_this(a)->value);
    while(a->p<a->file.s) {
        op=*par_this(a);
        if(!((op.type==STAR)||(op.type==SLASH))) {
            if(a->debug) printf("ne to! vmesto */ naydeno %s \r\n", op.value);
            break;
        }
        a->p++;
        AstNode *r=par_par_primary(a);
        l=(AstNode *)ast_create_binary(op.type, l, r);
    }
    return l;
}

AstNode *par_par_expr(Parser *a) {
    Token op, *t=par_this(a);
    AstNode *arg;
    if(a->debug) puts("par expr start");
    a->p++;
    switch(t->type) {
        case NATIVE: {
            Token *n=par_post(a);
            Func z;
            Type c;
            printf("library \'%s\', functions: \r\n", n->value);
            lib_add(a, n->value);
            expect(a, LPAREN, "\'{\'");
            do {
                n=par_post(a);
                if(n->type!=ID) error("Expected function name!");
                printf("%s ( ", n->value);
                skip(a, LBRACE);
                cv_init(&z.args, 4, sz(Type));
                z.name=strdup(n->value);
                do {
                    n=par_post(a);
                    c=par_stt(n->value);
                    cv_push(&z.args, &c);
                    printf("%s ", n->value);
                    if(par_this(a)->type==COMMA) a->p++;
                } while(par_this(a)->type!=RBRACE&&par_this(a)->type!=COLON);
                skip(a, RBRACE);
                printf(")\r\n");
                skip(a, COLON);
                n=par_post(a);
                z.ret_type=par_stt(n->value);
                func_add((Lib *)cv_back(&a->libs), z);
                if(par_this(a)->type==COMMA) a->p++;
            } while (par_this(a)->type!=RPAREN);
            expect(a, RPAREN, "\'}\'");
            return NULL;
        }
        case VAR: {
            AstStmtVarDecl *svd;
            Token *n, *w=par_post(a);
            AstNode *decl, *res=NULL;
            Variable *var;
            expect(a, COLON, ":");
            do {
                n=par_post(a);
                if(a->debug) printf("var; type=\'%s\', name=\'%s\' \r\n", w->value, n->value);
                var=scopes_addVar(&a->scopes, n->value, par_stt(w->value));
                decl=(AstNode*)ast_create_vardecl(n->value, par_stt(w->value), var->id);
                printf("var->id=%zu\n", var->id);
                if(!res) {
                    res=decl;
                } else {
                    res=(AstNode*)ast_create_comma(res, decl);
                }
            } while(par_this(a)->type==COMMA&&a->p++);
            return res;
        }
        case IF: {
            AstStmtBlock *body;
            AstNode *cond;
            cond=par_par_comp(a);
            body=par_parBlock(a);
            return (AstNode *)ast_create_if(cond, body, NULL);
        }
        case FUN: {
            char *name=par_post(a)->value;
            cv args;
            Type ret_type, c;
            AstStmtBlock *body;
            AstStmtFun *func;
            Token *n, *w;

            cv_init(&args, 4, sz(Type));
            size_t base_id=a->scopes.lastVar;
            scopes_enterScope(&a->scopes);
            printf("Function '%s' \r\n", name);
            if(par_this(a)->type==LBRACE) {
                if(par_next(a)->type!=RBRACE) {
                    do {
                        n=par_post(a);
                        expect(a, COLON, ":");
                        w=par_post(a);
                        c=par_stt(w->value);
                        cv_push(&args, &c);
                        printf("ARGUMENT: \'%s\'\r\n", par_tts(*(Type *)cv_back(&args)));
                        scopes_addVar(&a->scopes, n->value, c);
                        printf("type=\'%s\', name=\'%s\' \r\n", w->value, n->value);
                    } while(par_this(a)->type==COMMA&&a->p++);
                }
                skip(a, RBRACE);
            }
            if(par_this(a)->type==COLON) {
                expect(a, COLON, ":");
                n=par_post(a)->value;
                ret_type=par_stt(n);
            } else {
                n="Void";
                ret_type=T_VOID;
            }
            printf("return type: '%s' \r\n", n);
            body=par_parBlock(a);
            scopes_exitScope(&a->scopes);
            func=ast_create_fun(name, args, ret_type, body);
            func->base_id=base_id;
            cv_free(&args);
            return func;
        }

        case RETURN: {
            AstNode *value=par_par_comp(a);
            return (AstNode *)ast_create_ret(value);
        }

        case WHILE: {
            AstStmtBlock *body;
            AstNode *cond;
            cond=par_par_comp(a);
            body=par_parBlock(a);
            return (AstNode *)ast_create_while(cond, body);
        }

        case ID: {
            Token *w=par_this(a);
            if(w->type==LBRACE) {
                a->p--;
                cv q;
                Token *m=par_post(a);
                AstNode *n;
                cv_init(&q, 4, sizeof(AstNode *));
                if(a->debug) puts("lcall");
                expect(a, LBRACE, "\'(\'");
                if(par_this(a)->type!=RBRACE) {
                    do {
                        n=par_par_expr(a);
                        par_split_commas(n, &q);
                    } while(par_this(a)->type==COMMA&&a->p++);
                }
                expect(a, RBRACE, "\')\'");
                return (AstNode *)ast_create_call(m->value, q);
            } else {
                Variable *var;
                var=scopes_getVar(&a->scopes, t->value);
                if(!var) error("Expr: Undeclared variable: %s", t->value);
                if(w->type==ASSIGN) {
                    a->p++;
                    return ast_create_assign(t->value, par_par_comp(a), var->id, var->type);
                }
            }
        }
        default: a->p--;
    }
    AstNode *l=par_par_term(a);
    if(a->debug) puts("after calling term (in expr)");
    while(a->p<a->file.s) {
        op=*par_this(a);
        if(a->debug) printf("par_expr: op=\'%s\' \r\n", op.value);
        if(!((op.type==PLUS)||(op.type==MINUS))) {
            if(a->debug) printf("ne to! vmesto +- naydeno %s \r\n", op.value);
            break;
        }
        a->p++;
        AstNode *r=par_par_term(a);
        l=(AstNode *)ast_create_binary(op.type, l, r);
    }
    l=par_par_comma(a, l);
    return l;
}

AstNode *par_par_comma(Parser *a, AstNode *l) {
    while(a->p<a->file.s&&par_this(a)->type==COMMA) {
        a->p++;
        AstNode *r=par_par_comp(a);
        l=(AstNode*)ast_create_comma(l, r);
    }
    return l;
}

AstNode *par_par_comp(Parser *a) {
    AstNode *left=par_par_expr(a), *right;
    Token *op, *peek;
    while(a->p<a->file.s) {
        peek=par_this(a);
        switch(peek->type) {
            case EQ:
            case NEQ:
            case LT:
            case BT:
            case LE:
            case BE:
                break;
            default: return left;
        }
        op=par_post(a);
        right=par_par_comp(a);
        left=(AstNode *)ast_create_binary(op->type, left, right);
    }
    return left;
}
