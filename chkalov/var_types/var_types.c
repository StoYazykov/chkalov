#include "var_types.h"

Type par_stt(ds s) {
    size_t i;
    puts("called par_s2t");
    TypeNameEntry type_names[T_COUNT]={
        { "Unit",   T_VOID    },
        { "Void",   T_VOID    },
        { "Char",   T_CHAR    },
        { "Short",  T_SHORT   },
        { "Int",    T_INT     },
        { "Long",   T_LONG    },
        { "UChar",  T_UCHAR   },
        { "UShort", T_USHORT  },
        { "UInt",   T_UINT    },
        { "ULong",  T_ULONG   },
        { "String", T_STRING  },
    };

    for (i=0; i<T_COUNT; i++) {
        if (!strncmp(s, type_names[i].name, strlen(type_names[i].name)))
            return type_names[i].type;
    }
    printf("incorrect: %s\r\n", s);
    return T_INCORRECT;
}

char *par_tts(Type t) {
    switch(t) {
        case T_VOID: return "Void";
        case T_CHAR: return "Char";
        case T_INT: return "Int";
        case T_SHORT: return "Short";
        case T_LONG: return "Long";
        case T_UCHAR: return "UChar";
        case T_USHORT: return "UShort";
        case T_UINT: return "UInt";
        case T_ULONG: return "ULong";
        case T_STRING: return "String";
        default: {
            char *ptr=0x00595; // Seg
            *ptr=8;            // Fault
        }
    }
}

char par_ttc(Type t) {
    switch(t) {
        case T_VOID: return 'V';
        case T_CHAR: return 'C';
        case T_INT: return 'I';
        case T_SHORT: return 'X';
        case T_LONG: return 'L';
        case T_UCHAR: return 'c';
        case T_USHORT: return 'x';
        case T_UINT: return 'i';
        case T_ULONG: return 'l';
        case T_STRING: return 'S';
        default: error("par_t2s: incorrect type");
    }
}
