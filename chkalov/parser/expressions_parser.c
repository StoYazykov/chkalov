#include "expressions_parser.h"

AstNode *par_par_primary(Parser *a) {
    Token t=*par_this(a);
    if(a->debug) puts("par primary start");
    switch(t.type) {
        case NUMBER: {
            a->p++;
            return (AstNode *)ast_create_literal(NUMBER, t.value, t.suffix);
        }
        case STRING: {
            a->p++;
            return (AstNode *)ast_create_literal(STRING, t.value, SUFFIX_NONE);
        }
        case ID: {
            a->p++;
            if(par_this(a)->type==LBRACE) {
                a->p++;
                cv q;
                AstNode *n;
                cv_init(&q, 4, sizeof(AstNode *));
                if(par_this(a)->type!=RBRACE) {
                    do {
                        n=par_par_expr(a);
                        par_split_commas(n, &q);
                    } while(par_this(a)->type==COMMA&&a->p++);
                }
                expect(a, RBRACE, "\')\'");
                return (AstNode *)ast_create_call(t.value, q);
            }
            Variable *var=scopes_getVar(&a->scopes, t.value);
            if(!var) error("Primary: Undeclared variable \'%s\'", t.value);
            return (AstNode *)ast_create_variable(t.value, var->id, var->type);
        }
        case LBRACE: {
            a->p++;
            AstNode *p=par_par_expr(a);
            expect(a, RBRACE, "\')\'");
            return p;
        }
        default: error("Primary: unexpected token \'%s\'", t.value?t.value:"?");
    }
    return NULL;
}

AstNode *par_par_term(Parser *a) {
    Token op;
    if(a->debug) puts("par term start");
    AstNode *l=par_par_primary(a);
    while(a->p<a->file.s) {
        op=*par_this(a);
        if(!((op.type==STAR)||(op.type==SLASH))) break;
        a->p++;
        AstNode *r=par_par_primary(a);
        l=(AstNode *)ast_create_binary(op.type, l, r);
    }
    return l;
}

AstNode *par_par_add(Parser *a) {
    Token op;
    AstNode *l=par_par_term(a);
    while(a->p<a->file.s) {
        op=*par_this(a);
        if(!((op.type==PLUS)||(op.type==MINUS))) break;
        a->p++;
        AstNode *r=par_par_term(a);
        l=(AstNode *)ast_create_binary(op.type, l, r);
    }
    return l;
}

AstNode *par_par_cmp(Parser *a) {
    AstNode *l=par_par_add(a), *r;
    Token *op;
    while(a->p<a->file.s) {
        op=par_this(a);
        switch(op->type) {
            case EQ:
            case NEQ:
            case LT:
            case BT:
            case LE:
            case BE:
                break;
            default: return l;
        }
        a->p++;
        r=par_par_add(a);
        l=(AstNode *)ast_create_binary(op->type, l, r);
    }
    return l;
}

AstNode *par_par_expr(Parser *a) {
    AstNode *l=par_par_cmp(a);
    if(a->p<a->file.s&&par_this(a)->type==ASSIGN) {
        a->p++;
        if(l->type!=AST_EXPR_VARIABLE) error("Assign: left not variable");
        AstExprVariable *v=(AstExprVariable *)l;
        AstNode *r=par_par_expr(a);
        return ast_create_assign(v->name, r, v->id, v->type);
    }
    return l;
}

AstNode *par_par_comma(Parser *a, AstNode *l) {
    while(a->p<a->file.s&&par_this(a)->type==COMMA) {
        a->p++;
        AstNode *r=par_par_expr(a);
        l=(AstNode*)ast_create_comma(l, r);
    }
    return l;
}

AstNode *par_par_stmt(Parser *a) {
    Token t=*par_this(a);
    switch(t.type) {
        case IMPORT: {
            Token *n;
            a->p++;
            n=par_post(a);
            if(n->type!=STRING) error("Expected module name string!");
            printf("import: %s\n", n->value);
            par_import_module(a, n->value);
            return NULL;
        }
        case NATIVE: {
            Token *n;
            Func z;
            Type c;
            a->p++;
            n=par_this(a);
            a->p++;
            if(n->type!=ID) error("Expected library name!");
            printf("library '%s', functions:\r\n", n->value);
            lib_add(a, n->value);
            expect(a, LPAREN, "'{'");
            do {
                n=par_this(a);
                a->p++;
                if(n->type!=ID) error("Expected function name!");
                printf("%s ( ", n->value);
                skip(a, LBRACE);
                cv_init(&z.args, 4, sz(Type));
                z.name=strdup(n->value);
                do {
                    n=par_this(a);
                    a->p++;
                    c=par_stt(n->value);
                    cv_push(&z.args, &c);
                    printf("%s ", n->value);
                    if(par_this(a)->type==COMMA) a->p++;
                } while(par_this(a)->type!=RBRACE&&par_this(a)->type!=COLON);
                skip(a, RBRACE);
                printf(")\r\n");
                skip(a, COLON);
                n=par_this(a);
                a->p++;
                z.ret_type=par_stt(n->value);
                func_add((Lib *)cv_back(&a->libs), z);
                if(par_this(a)->type==COMMA) a->p++;
            } while(par_this(a)->type!=RPAREN);
            expect(a, RPAREN, "'}'");
            return NULL;
        }
        case VAR: {
            Token *n, *w;
            AstNode *decl, *res=NULL;
            Variable *var;
            a->p++;
            w=par_this(a);
            a->p++;
            expect(a, COLON, ":");
            do {
                n=par_this(a);
                a->p++;
                if(a->debug) printf("var; type='%s', name='%s'\r\n", w->value, n->value);
                var=scopes_addVar(&a->scopes, n->value, par_stt(w->value));
                decl=(AstNode*)ast_create_vardecl(n->value, par_stt(w->value), var->id);
                if(!res) res=decl;
                else res=(AstNode*)ast_create_comma(res, decl);
            } while(par_this(a)->type==COMMA&&a->p++);
            return res;
        }
        case IF: {
            AstStmtBlock *body;
            AstNode *cond;
            a->p++;
            cond=par_par_cmp(a);
            body=par_parBlock(a);
            return (AstNode *)ast_create_if(cond, body, NULL);
        }
        case FUN: {
            char *name;
            cv args;
            Type ret_type, c;
            AstStmtBlock *body;
            AstStmtFun *func;
            Token *n, *w;
            a->p++;
            n=par_this(a);
            a->p++;
            name=n->value;
            cv_init(&args, 4, sz(Type));
            size_t base_id=a->scopes.lastVar;
            scopes_enterScope(&a->scopes);
            printf("Function '%s'\r\n", name);
            if(par_this(a)->type==LBRACE) {
                if(par_next(a)->type!=RBRACE) {
                    do {
                        n=par_this(a);
                        a->p++;
                        expect(a, COLON, ":");
                        w=par_this(a);
                        a->p++;
                        c=par_stt(w->value);
                        cv_push(&args, &c);
                        scopes_addVar(&a->scopes, n->value, c);
                    } while(par_this(a)->type==COMMA&&a->p++);
                }
                skip(a, RBRACE);
            }
            if(par_this(a)->type==COLON) {
                expect(a, COLON, ":");
                n=par_this(a);
                a->p++;
                ret_type=par_stt(n->value);
            } else {
                ret_type=T_VOID;
            }
            printf("return type: '%s'\r\n", par_tts(ret_type));
            body=par_parBlock(a);
            scopes_exitScope(&a->scopes);
            func=ast_create_fun(name, args, ret_type, body);
            func->base_id=base_id;
            cv_free(&args);
            return (AstNode *)func;
        }
        case RETURN: {
            AstNode *value;
            a->p++;
            value=par_par_cmp(a);
            return (AstNode *)ast_create_ret(value);
        }
        case WHILE: {
            AstStmtBlock *body;
            AstNode *cond;
            a->p++;
            cond=par_par_cmp(a);
            body=par_parBlock(a);
            return (AstNode *)ast_create_while(cond, body);
        }
        default: {
            return par_par_expr(a);
        }
    }
}
