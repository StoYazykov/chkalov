#ifndef CHKALOV_H
#define CHKALOV_H

#ifndef unix
#define unix
#endif

#ifdef __cplusplus
#include <cstdint>
#include <sstream>
#include <stdexcept>
#endif
#include <dlfcn.h>

#define error(errm) do{ \
    ostringstream oss; \
    oss << errm; \
    throw ChkalovError(oss.str()); \
}while(0);

#ifdef __cplusplus
#include <string>
#else
#include <string.h>
#endif

#ifdef __cplusplus
using namespace std;
#endif

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

// ----------------------------------------------//
//            Sizes definitions!                 //
// ----------------------------------------------//

#define CHAR 0x00   // 1 bytes
#define SHORT 0x01  // 2 bytes
#define INT 0x02    // 4 bytes
#define LONG 0x03   // 8 bytes
#define ADDR 0x04
#define ARRAY 0x05  // ? bytes
#define LIBRARY 0x06
#define STR 0x07
#define NOL 0x08

#define CHARU 0x08
#define SHORTU 0x09
#define INTU 0x0A
#define LONGU 0x0B

#define getGID (nullscope.getGlobalId())
#define _T(type) (type)
#define _P (*)
#define _CP (char _P)
#define _TP(type) (type _P)
#define _TPL (char*)&
#define sz(type) (sizeof(type))

#pragma pack(push, 1)

#ifdef __cplusplus
template<typename T>
T pop(vector<T>& v) {
    T val = v.back();
    v.pop_back();
    return val;
}
#endif // __cplusplus

//              CHAR SHORT INT LONG ADDR ARRAY LIBRARY STR CHARU SHORTU INTU LONGU NOL NULL
const int st[]={1,   2,    4,  8,   8,   0xFF, 4,      4,  1,    2,     4,   8,    0,  0};  // sizes table

// ----------------------------------------------//
//            VM instruction structure           //
// ----------------------------------------------//

#ifdef __cplusplus
struct Vm {
#else
typedef struct {
#endif // __cplusplus
    unsigned char opcode;
    unsigned char type;
    int64_t value; // 64 bits - maximal size, will truncated in Parser::finally(). //
#ifdef __cplusplus
};
#else
} Vm;
#endif // __cplusplus

#ifdef __cplusplus
struct Slot {
#else
typedef struct {
#endif // __cplusplus
    unsigned char type;
    int64_t value;
#ifdef __cplusplus
};
#else
} Slot;
#endif // __cplusplus

// ----------------------------------------------//
//              Pool entry structure!            //
// ----------------------------------------------//

#ifdef __cplusplus
struct Pool {
    unsigned char type;
    string value;
};
#endif

/*-128 - 127 CHAR
0-255 CHARU
-32768 - 32767 SHORT
0-65536 SHORTU
-2^32/2 - 2^32/2-1 INT
0-2^32 INTU
-2^64/2 - 2^64/2-1 LONG
0-2^64 LONGU*/

#ifdef __cplusplus
inline
#endif
uint8_t seltypeu(uint64_t a){
    if(a<256) return CHARU;
    if(a<65536) return SHORTU;
    if(a<(1ULL<<32)) return INTU;
    return LONGU;
}

#ifdef __cplusplus
class ChkalovError : public runtime_error {
public:
    ChkalovError(const string& msg) : runtime_error(msg) {}
};
#endif

#ifdef __cplusplus
inline
#endif
uint8_t seltype(int64_t a){
    if(a>=-128&&a<128) return CHAR;
    if(a>=-32768&&a<32768) return SHORT;
    if(a>=-((1LL<<32)/2)&&a<((1LL<<32)/2)) return INT;
    return LONG;
}
#endif
