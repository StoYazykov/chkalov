#ifndef _VAR_TYPES_H
#define _VAR_TYPES_H

#include "../../chkalov.h"

typedef enum {
    T_INCORRECT=0x00,
    T_VOID=0x01,
    T_CHAR=0x02,
    T_SHORT=0x03,
    T_INT=0x04,
    T_LONG=0x05,
    T_UCHAR=0x06,
    T_USHORT=0x07,
    T_UINT=0x08,
    T_ULONG=0x09,
    T_STRING=0x0A,
    T_COUNT=0x0B
} Type;

typedef struct {
    char *name;
    Type type;
} TypeNameEntry;

Type par_stt(ds s);
char *par_tts(Type t);
char par_ttc(Type t);

#endif
