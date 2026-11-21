#ifndef _ANALYZER_H
#define _ANALYZER_H

#include "../parser/parser.h"
#include "../ast/ast.h"
#include "../var_types/var_types.h"
#include "../scope/scope.h"

void sem_analyze(AstNode *node, Parser *a);
Type sem_binary_type(TokType op, Type left_type, Type right_type);
bool sem_types_compatible(Type left_type, Type right_type);
void collect_funcs(AstNode *node, Parser *a);

static Type binary_result[5][T_COUNT][T_COUNT]={
{
{ T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_CHAR,      T_SHORT,     T_INT,       T_LONG,      T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_SHORT,     T_SHORT,     T_INT,       T_LONG,      T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_INT,       T_INT,       T_INT,       T_LONG,      T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_LONG,      T_LONG,      T_LONG,      T_LONG,      T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_STRING    },
},
{
{ T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_CHAR,      T_SHORT,     T_INT,       T_LONG,      T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_SHORT,     T_SHORT,     T_INT,       T_LONG,      T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_INT,       T_INT,       T_INT,       T_LONG,      T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_LONG,      T_LONG,      T_LONG,      T_LONG,      T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT },
},
{
{ T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_CHAR,      T_SHORT,     T_INT,       T_LONG,      T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_SHORT,     T_SHORT,     T_INT,       T_LONG,      T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_INT,       T_INT,       T_INT,       T_LONG,      T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_LONG,      T_LONG,      T_LONG,      T_LONG,      T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT },
},
{
{ T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_CHAR,      T_SHORT,     T_INT,       T_LONG,      T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_SHORT,     T_SHORT,     T_INT,       T_LONG,      T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_INT,       T_INT,       T_INT,       T_LONG,      T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_LONG,      T_LONG,      T_LONG,      T_LONG,      T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT },
},
{
{ T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_INT,       T_INT,       T_INT,       T_INT,       T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_INT,       T_INT,       T_INT,       T_INT,       T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_INT,       T_INT,       T_INT,       T_INT,       T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_INT,       T_INT,       T_INT,       T_INT,       T_INCORRECT },
{ T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INT       },
},
};

static bool types_compatible[T_COUNT][T_COUNT]={
{ false, false, false, false, false, false, false },
{ false, true,  false, false, false, false, false },
{ false, false, true,  true,  true,  true,  false },
{ false, false, false, true,  true,  true,  false },
{ false, false, false, false, true,  true,  false },
{ false, false, false, false, false, true,  false },
{ false, false, false, false, false, false, true  },
};

#endif
