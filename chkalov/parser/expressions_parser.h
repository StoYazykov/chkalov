#ifndef _EXPRESSIONS_PARSER_H
#define _EXPRESSIONS_PARSER_H

#include "parser.h"

AstNode *par_par_primary(Parser *a);
AstNode *par_par_term(Parser *a);
AstNode *par_par_expr(Parser *a);
AstNode *par_par_comma(Parser *a, AstNode *l);
AstNode *par_par_comp(Parser *a);

//void pa

#endif
