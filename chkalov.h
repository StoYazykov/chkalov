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

#define MAX(a, b) ((a)>(b)?(a):(b))


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

#define error(...) (fprintf(stderr, "Error: " __VA_ARGS__),fprintf(stderr, "!\r\n"),exit(0x08),0)

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "dynamic-strings/ds.h"
#include "c-vectors/cv.h"

// ----------------------------------------------//
//              VM opcodes definitions           //
// ----------------------------------------------//

#define OP_CPUSH      0x00
#define OP_SPUSH      0x01
#define OP_IPUSH      0x02
#define OP_LPUSH      0x03
#define OP_APUSH      0x04
#define OP_POP        0x05
#define OP_DUP        0x06
#define OP_SWAP       0x07
#define OP_JUMP       0x08
#define OP_JMP_IF     0x09
#define OP_JMP_IFN    0x0A
#define OP_CALL       0x0B
#define OP_RET        0x0C

#define OP_ARG        0x0D
#define OP_INVOKE     0x0E
#define OP_INVOKE_EXT 0x0F
#define OP_ALLOC      0x10
#define OP_FREE       0x11 // base end

#define OP_CLOAD      0x12 // all 4 bytes start
#define OP_SLOAD      0x13
#define OP_ILOAD      0x14
#define OP_LLOAD      0x15
#define OP_ALOAD      0x16
#define OP_CSTORE     0x17
#define OP_SSTORE     0x18
#define OP_ISTORE     0x19
#define OP_LSTORE     0x1A
#define OP_ASTORE     0x1B // all 4 bytes end

// Char
#define OP_CADD       0x1C // all 0 bytes (no-operand) start
#define OP_CSUB       0x1D
#define OP_CMUL       0x1E
#define OP_CDIV       0x1F
#define OP_CMOD       0x20
// Short
#define OP_SADD       0x21
#define OP_SSUB       0x22
#define OP_SMUL       0x23
#define OP_SDIV       0x24
#define OP_SMOD       0x25
// Int
#define OP_IADD       0x26
#define OP_ISUB       0x27
#define OP_IMUL       0x28
#define OP_IDIV       0x29
#define OP_IMOD       0x2A
// Long
#define OP_LADD       0x2B
#define OP_LSUB       0x2C
#define OP_LMUL       0x2D
#define OP_LDIV       0x2E
#define OP_LMOD       0x2F
// Address
#define OP_AADD       0x30
#define OP_ASUB       0x31

// Char
#define OP_CEQ        0x32
#define OP_CNEQ       0x33
#define OP_CLT        0x34
#define OP_CBT        0x35
#define OP_CLE        0x36
#define OP_CBE        0x37
// Short
#define OP_SEQ        0x38
#define OP_SNEQ       0x39
#define OP_SLT        0x3A
#define OP_SBT        0x3B
#define OP_SLE        0x3C
#define OP_SBE        0x3D
// Int
#define OP_IEQ        0x3E
#define OP_INEQ       0x3F
#define OP_ILT        0x40
#define OP_IBT        0x41
#define OP_ILE        0x42
#define OP_IBE        0x43
// Long
#define OP_LEQ        0x44
#define OP_LNEQ       0x45
#define OP_LLT        0x46
#define OP_LBT        0x47
#define OP_LLE        0x48
#define OP_LBE        0x49
// Address
#define OP_AEQ        0x4A
#define OP_ANEQ       0x4B
#define OP_ALT        0x4C
#define OP_ABT        0x4D
#define OP_ALE        0x4E
#define OP_ABE        0x4F

// Char
#define OP_CAND       0x50
#define OP_COR        0x51
#define OP_CXOR       0x52
#define OP_CNOT       0x53
// Short
#define OP_SAND       0x54
#define OP_SOR        0x55
#define OP_SXOR       0x56
#define OP_SNOT       0x57
// Int
#define OP_IAND       0x58
#define OP_IOR        0x59
#define OP_IXOR       0x5A
#define OP_INOT       0x5B
// Long
#define OP_LAND       0x5C
#define OP_LOR        0x5D
#define OP_LXOR       0x5E
#define OP_LNOT       0x5F
// Address
#define OP_AAND       0x60
#define OP_AOR        0x61
#define OP_AXOR       0x62
#define OP_ANOT       0x63

#define OP_CSHL       0x64
#define OP_CSHR       0x65
#define OP_SSHL       0x66
#define OP_SSHR       0x67
#define OP_ISHL       0x68
#define OP_ISHR       0x69
#define OP_LSHL       0x6A
#define OP_LSHR       0x6B
#define OP_ASHL       0x6C
#define OP_ASHR       0x6D

#define OP_CNEG       0x6E
#define OP_SNEG       0x6F
#define OP_INEG       0x70
#define OP_LNEG       0x71
#define OP_ANEG       0x72 // all 0 bytes (no-operand) end

#define OP_CAST       0x73

#define OP_COUNT      0x74


// ----------------------------------------------//
//            Types definitions                  //
// ----------------------------------------------//

#define sz(type) (sizeof(type))
#define NONE ((size_t)-1)

#pragma pack(push, 1)


// ----------------------------------------------//
//            VM instruction structure           //
// ----------------------------------------------//

typedef struct {
    unsigned char opcode;
    int64_t value; // 64 bits - maximal size, will truncated in par_free(a). //
} Vm;

typedef struct {
    uint32_t magic;
    uint64_t entry_point;
    uint64_t heap_size;
    uint64_t pool_size;
    uint64_t code_size;
    char reserved[16];
} Header;

uint8_t selsz(int64_t a);
uint8_t selszu(uint64_t a);

#pragma pack(pop)

#endif
