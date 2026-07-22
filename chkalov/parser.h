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
    bool debug;
    char lit;
    cv code, file;
    char *heap;
    size_t hp, hs;
    cv imports;
    AstProgram *root;
    ScopeStack scopes;
    size_t p;
} Parser;

void par_init(Parser *a, const ds fni, const ds fno, bool deb);
size_t par_heapIns(Parser *a, ds k);
void par_free(Parser *a);
Token* par_this(Parser *a);
Token* par_next(Parser *a);
void par_parFile(Parser *a);
void expect(Parser *a, TokenType t, char *s);

#endif
