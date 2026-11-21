#ifndef _EXPRESSIONS_PARSER_H
#define _EXPRESSIONS_PARSER_H

#include "parser.h"

AstNode *par_par_primary(Parser *a);
AstNode *par_par_term(Parser *a);
AstNode *par_par_add(Parser *a);
AstNode *par_par_cmp(Parser *a);
AstNode *par_par_expr(Parser *a);
AstNode *par_par_stmt(Parser *a);
AstNode *par_par_unary(Parser *a);

AstNode *par_par_comma(Parser *a, AstNode *l);
AstStmtBlock *par_parBlock(Parser *a);
void par_split_commas(AstNode *n, cv *q);

#endif
