#ifndef _ANALYZER_H
#define _ANALYZER_H

#include "../parser/parser.h"
#include "../ast/ast.h"

void sem_analyze(AstNode *node, Parser *a);
uint8_t sem_binary_type(TokType op, uint8_t lt, uint8_t rt);
bool sem_types_compatible(uint8_t lt, uint8_t rt);
void collect_funcs(AstNode *node, Parser *a);

#endif _ANALYZER_H
