#ifndef GETTIMEOFDAY_H
#define GETTIMEOFDAY_H

#include < time.h >
#include <winsock2.h>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif
 
int gettimeofday(struct timeval *tv, void *p);

void timersub(struct timeval *a, struct timeval *b, struct timeval *res);

#endif