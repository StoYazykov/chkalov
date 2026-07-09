#ifndef CHKALOV_H
#define CHKALOV_H

#define _POSIX_C_SOURCE 200809L

#define ISSTR(a) ((a&0xf0)==STR)
#define ISNUM(a) (!ISSTR(a))
#define CSUB(a,b,c) (!strncmp(a, b, c))
#define IFCS(a,b,c) if(CSUB(a,b,c))
#define SEQU(a,b) (!strcmp(a,b))

#ifdef _WIN32
    #include <windows.h>
    #define RTLD_LAZY 0
    void* dlopen(const char* p, int m) {
        return LoadLibrary(p);
    }
    void* dlsym(void* h, const char* n) {
        return GetProcAddress(h, n);
    }
    void dlclose(void* h) {
        FreeLibrary(h);
    }
#else
    #include <dlfcn.h>
#endif

#define error(...)do{\
fprintf(stderr, "Error: ");\
fprintf(stderr, __VA_ARGS__);\
fprintf(stderr, "!\r\n");\
exit(0x08);\
}while(0)

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "ds.h"
#include "cv.h"
// ----------------------------------------------//
//              VM opcodes definitions           //
// ----------------------------------------------//

// 0x00..0x07 opcodes is reserved.

#define LOAD 0x08
#define PUSH 0x09
#define STORE 0x0A
#define INC 0x0B
#define DEC 0x0C
#define ADD 0x0D
#define SUB 0x0E
#define MUL 0x0F
#define DIV 0x10

#define CALL 0x11

#define ALLOC 0x12

#define IFE 0x13    // if equal ==
#define IFNE 0x14   // if not equal !=
#define IFL 0x15    // if less <
#define IFB 0x16    // if big >
#define IFBE 0x17   // if big or equal >=
#define IFLE 0x18   // if less or equal <=

#define SETFIELD 0x19
#define GETFIELD 0x1A

// ----------------------------------------------//
//            Types definitions!                 //
// ----------------------------------------------//

#define CHAR 0x00
#define XSHORT 0x10
#define SHORT 0x20
#define INT 0x30
#define LONG 0x40
#define STR 0x50
#define LIBRARY 0x60
#define PTR 0x70
#define NOL 0x80
#define IDX 0x90

#define sz(type) (sizeof(type))

#pragma pack(push, 1)


// ----------------------------------------------//
//            VM instruction structure           //
// ----------------------------------------------//

typedef struct {
    unsigned char opcode;
    unsigned char type;
    int64_t value; // 64 bits - maximal size, will truncated in par_free(a). //
} Vm;

typedef struct {
    unsigned char type;
    int64_t value;
} Slot;

// ----------------------------------------------//
//              Pool entry structure!            //
// ----------------------------------------------//

typedef struct {
    unsigned char type;
    ds value;
} Pool;

uint8_t selstrt(uint64_t a);
uint8_t selsz(int64_t a);

#endif
