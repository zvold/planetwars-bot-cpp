#include "utils.h"

uint8_t nlz(uint64_t x) {
    if (x == 0)
        return 64;

    uint8_t n = 0;
    if (x <= 0x00000000ffffffffLL) {n += 32; x <<= 32;}
    if (x <= 0x0000ffffffffffffLL) {n += 16; x <<= 16;}
    if (x <= 0x00ffffffffffffffLL) {n +=  8; x <<=  8;}
    if (x <= 0x0fffffffffffffffLL) {n +=  4; x <<=  4;}
    if (x <= 0x3fffffffffffffffLL) {n +=  2; x <<=  2;}
    if (x <= 0x7fffffffffffffffLL) {n +=  1;}

    return n;
}

