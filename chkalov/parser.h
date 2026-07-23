#ifndef PARSER_H
#define PARSER_H

#include "scope.h"
#include "ast.h"
#include "token.h"
#include "../chkalov.h"

typedef struct {
    cv funcs;
    ds name;
} Import;

bool imp_cont(Import *a, ds b);
void imp_add(Import *a, ds z);

typedef struct {
    ds fn;
    int debug;
    cv code, file;
    char *heap;
    size_t hp, hs;
    cv imports;
    AstStmtBlock *root;
    ScopeStack scopes;
    size_t p;
} Parser;

void par_init(Parser *a, const ds fni, const ds fno, int deb);
size_t par_heapIns(Parser *a, ds k);
void par_free(Parser *a);
Token* par_this(Parser *a);
Token* par_next(Parser *a);
void par_render(Parser *a, uint8_t op, uint8_t ty, int64_t v);
void par_parFile(Parser *a);
void expect(Parser *a, TokenType t, char *s);
AstNode *par_par_primary(Parser *a);
AstNode *par_par_term(Parser *a);
AstNode *par_par_expr(Parser *a);
void par_optim(Parser *a);

#endif
