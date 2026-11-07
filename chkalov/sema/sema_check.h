#ifndef _ANALYZER_H
#define _ANALYZER_H

#include "../parser/parser.h"
#include "../ast/ast.h"
#include "../var_types/var_types.h"

void sem_analyze(AstNode *node, Parser *a);
Type sem_binary_type(TokType op, Type left_type, Type right_type);
//bool sem_types_compatible(Type left_type, Type right_type);
void collect_funcs(AstNode *node, Parser *a);

static Type binary_result[T_COUNT][T_COUNT]={
/*                  T_INCORRECT  T_VOID       T_CHAR       T_SHORT      T_INT        T_LONG       T_UCHAR      T_USHORT     T_UINT       T_ULONG      T_STRING   */
/* T_INCORRECT */ { T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT },
/* T_VOID      */ { T_INCORRECT, T_VOID,      T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT },
/* T_CHAR      */ { T_INCORRECT, T_INCORRECT, T_CHAR,      T_SHORT,     T_INT,       T_LONG,      T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT },
/* T_SHORT     */ { T_INCORRECT, T_INCORRECT, T_INCORRECT, T_SHORT,     T_INT,       T_LONG,      T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT },
/* T_INT       */ { T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INT,       T_LONG,      T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT },
/* T_LONG      */ { T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_LONG,      T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT },
/* T_UCHAR     */ { T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_UCHAR,     T_USHORT,    T_UINT,      T_ULONG,     T_INCORRECT },
/* T_USHORT    */ { T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_USHORT,    T_UINT,      T_ULONG,     T_INCORRECT },
/* T_UINT      */ { T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_UINT,      T_ULONG,     T_INCORRECT },
/* T_ULONG     */ { T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_ULONG,     T_INCORRECT },
/* T_STRING    */ { T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_INCORRECT, T_STRING    }
};

#ifdef _A_A
static bool types_compatible[T_COUNT][T_COUNT]={
/*                   T_INCORRECT T_VOID T_CHAR T_SHORT T_INT  T_LONG T_UCHAR T_USHORT T_UINT T_ULONG T_STRING */
/* T_INCORRECT */  { false,      false, false, false,  false, false, false,  false,   false, false,  false },
/* T_VOID      */  { false,      true,  false, false,  false, false, false,  false,   false, false,  false },
/* T_CHAR      */  { false,      false, true,  true,   true,  true,  false,  false,   false, false,  false },
/* T_SHORT     */  { false,      false, false, false,  false, false, false,  false,   false, false,  false },
/* T_INT       */  { false,      false, false, false,  false, false, false,  false,   false, false,  false },
/* T_LONG      */  { false,      false, false, false,  false, false, false,  false,   false, false,  false },
/* T_UCHAR     */  { false,      false, false, false,  false, false, false,  false,   false, false,  false },
/* T_USHORT    */  { false,      false, false, false,  false, false, false,  false,   false, false,  false },
/* T_UINT      */  { false,      false, false, false,  false, false, false,  false,   false, false,  false },
/* T_ULONG     */  { false,      false, false, false,  false, false, false,  false,   false, false,  false },
/* T_STRING    */  { false,      false, false, false,  false, false, false,  false,   false, false,  false },
}

#endif
#endif _ANALYZER_H
