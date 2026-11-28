#include "var_types.h"

Type par_stt(ds s) {
    size_t i;
    fputs("called par_s2t\n", stderr);
    TypeNameEntry type_names[T_COUNT]={
        { "Unit",   T_VOID    },
        { "Void",   T_VOID    },
        { "Char",   T_CHAR    },
        { "Short",  T_SHORT   },
        { "Int",    T_INT     },
        { "Long",   T_LONG    },
        { "String", T_STRING  },
    };

    for (i=0; i<T_COUNT; i++) {
        if (!strncmp(s, type_names[i].name, strlen(type_names[i].name)))
            return type_names[i].type;
    }
    fprintf(stderr, "incorrect: %s\r\n", s);
    return T_INCORRECT;
}

Type par_ctt(char c) {
    switch(c) {
        case 'V': return T_VOID;
        case 'C': return T_CHAR;
        case 'I': return T_INT;
        case 'X': return T_SHORT;
        case 'L': return T_LONG;
        case 'S': return T_STRING;
        default: error("par_ctt: incorrect type \'%c\', %x", c, c);
    }
}

char *par_tts(Type t) {
    switch(t) {
        case T_VOID: return "Void";
        case T_CHAR: return "Char";
        case T_INT: return "Int";
        case T_SHORT: return "Short";
        case T_LONG: return "Long";
        case T_STRING: return "String";
        default: {
            *((volatile char *)0)=0;
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
        case T_STRING: return 'S';
        default: error("par_ttc: incorrect type");
    }
}
