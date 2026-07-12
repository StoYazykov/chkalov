#ifndef PARSER_H
#define PARSER_H

#include "scope.h"
#include "token.h"
#include "../chkalov.h"

typedef struct {
    cv funcs;
    ds name;
} Import;

void imp_init(Import *a, ds n);
bool imp_cont(Import *a, ds b);
void imp_add(Import *a, ds z);
void imp_free(Import *a);

typedef struct {
    ds fn;
    bool debug;
    char lit;
    cv code;
    char *heap;
    size_t hp, hs;
    cv imports;
    cv file;
    ScopeStack scopes;
    size_t p;
} Parser;

void par_init(Parser *a, const ds fni, const ds fno, bool deb);
void par_par(Parser *a, const ds l);
void par_parBlock(Parser *a);
void par_parExpr(Parser *a);
void par_parTerm(Parser *a);
void par_parFact(Parser *a);
void par_parIf(Parser *a);
size_t par_heapIns(Parser *a, ds k);
Token par_next(Parser *a);
Token par_nexti(Parser *a);
void par_parIns(Parser *a);
char par_stt(const ds s);
void par_render(Parser *a, Vm vm);
void par_parFile(Parser *a);
void par_free(Parser *a);

#endif
