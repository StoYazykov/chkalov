#include "chkalov.h"

uint8_t selstrt(uint64_t a){
    if(a<256) return STR|0x01;
    if(a<65536) return STR|0x02;
    if(a<(1ULL<<32)) return STR|0x04;
    return STR|0x08;
}

uint8_t selsz(int64_t a) {
    if(a<256) return 0x01;
    if(a<65536) return 0x02;
    if(a<(1ULL<<32)) return 0x04;
    return 0x08;
}
