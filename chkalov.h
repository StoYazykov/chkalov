/*
 * Copyright (C) 2025-2026  StoYazykov
 *
 * This file is part of Chkalov.
 *
 * Chkalov is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Chkalov is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Chkalov.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CHKALOV_H
#define CHKALOV_H

#define VERSION "0.1.0"

#define _POSIX_C_SOURCE 200809L

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

#define error(...) (fprintf(stderr, "Error: %s!\r\n" __VA_ARGS__),exit(0x08),0)

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

#define CMP_EQ 0x13    // if equal ==
#define CMP_NEQ 0x14   // if not equal !=
#define CMP_LT 0x15    // if less <
#define CMP_BT 0x16    // if big >
#define CMP_BE 0x17   // if big or equal >=
#define CMP_LE 0x18   // if less or equal <=
#define JUMP 0x19

#define SETFIELD 0x1A
#define GETFIELD 0x1B

#define DUP 0x1C

#define JMP_IF 0x1D
#define JMP_IFN 0x1E


// ----------------------------------------------//
//            Types definitions!                 //
// ----------------------------------------------//

#define CHAR 0x01
#define XSHORT 0x02
#define SHORT 0x03
#define INT 0x04
#define LONG 0x05
#define STR 0x06 // string in HEAP
#define LIBRARY STR
#define PTR 0x07
#define IDX 0x08

#define sz(type) (sizeof(type))

#pragma pack(push, 1)


// ----------------------------------------------//
//            VM instruction structure           //
// ----------------------------------------------//

typedef struct {
    unsigned char opcode;
    int64_t value; // 64 bits - maximal size, will truncated in par_free(a). //
} Vm;

uint8_t selsz(int64_t a);
uint8_t selszu(uint64_t a);

#endif
