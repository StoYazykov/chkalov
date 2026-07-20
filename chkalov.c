#include "chkalov.h"

uint8_t selszu(uint64_t a) {
    if(!a) return 0;
    if(a<256) return 0x01;
    if(a<65536) return 0x02;
    if(a<(1ULL<<32)) return 0x04;
    return 0x08;
}

uint8_t selsz(int64_t a) {
    if(!a) return 0;
    if(a>-128&&a<128) return 0x01;
    if(a>-32768&&a<32768) return 0x02;
    if(a>-(1LL<<31)&&a<(1LL<<31)) return 0x04;
    return 0x08;
}
