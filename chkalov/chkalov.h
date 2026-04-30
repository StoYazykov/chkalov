#ifndef CHKALOV_H
#define CHKALOV_H

#ifndef unix
#define unix
#endif

#include <cstdint>
#include <dlfcn.h>

#define error(errm) do{cerr<<"Error! "<<errm<<endl;exit(0x08);}while(0);

#include <string>

using namespace std;

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

// ----------------------------------------------//
//            Sizes definitions!                 //
// ----------------------------------------------//

#define CHAR 0x00   // 1 bytes
#define SHORT 0x01  // 2 bytes
#define INT 0x02    // 4 bytes
#define LONG 0x03   // 8 bytes
#define ADDR 0x04
#define ARRAY 0x05  // ? bytes

#define CALL 0x11

#define IFE 0x12    // if equal ==
#define IFNE 0x13   // if not equal !=
#define IFL 0x14    // if less <
#define IFB 0x15    // if big >
#define IFBE 0x16   // if big or equal >=
#define IFLE 0x17   // if less or equal <=

// ----------------------------------------------//
//                  Pool types!                  //
// ----------------------------------------------//

#define LIBRARY LONG
#define STR LONG

#define getGID (nullscope.getGlobalId())
#define _T(type) (type)
#define _P (*)
#define _CP (char _P)
#define _TP(type) (type _P)
#define _TPL (char*)&
#define sz(type) (sizeof(type))

#pragma pack(push, 1)

template<typename T>
T pop(vector<T>& v) {
    T val = v.back();
    v.pop_back();
    return val;
}

const int st[]={1,2,4,8,8};  // sizes table

// ----------------------------------------------//
//            VM instruction structure           //
// ----------------------------------------------//

struct Vm {
    unsigned char opcode;
    char type;
    int64_t value; // 64 bits - maximal size, will truncated in Parser::finally(). //
};

// ----------------------------------------------//
//              Pool entry structure!            //
// ----------------------------------------------//

struct Pool {
    unsigned char type;
    string value;
};

#pragma pack(pop)

inline uint8_t seltype(int64_t a){
    if(a<256) return CHAR;
    if(a<65536) return SHORT;
    if(a<(1ULL<<32)) return INT;
    return LONG;
}

#endif
