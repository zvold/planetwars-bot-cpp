#ifndef UTILS_H
#define UTILS_H

#ifndef MY_DEBUG
#ifndef NDEBUG
#define NDEBUG
#endif
#endif

#include <assert.h>

#ifdef WIN32
#include "pstdint.h"
#else
#include <stdint.h>
#endif

#include <string>
#include <sstream>

#define TYPICAL_MAPSIZE 24

// planet id type definition, 64 planets allowed
typedef uint16_t plid_t;
typedef uint64_t pmask_t;

typedef uint16_t ships_t;
typedef uint16_t turn_t;

template <typename T>
std::string str(T const &from) {
   std::ostringstream o;
   o << from;
   return o.str();
}

uint8_t nlz(uint64_t x);

#endif // UTILS_H
