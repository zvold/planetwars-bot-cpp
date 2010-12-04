#ifdef WIN32

#include "gettimeofday.h"
 
int gettimeofday(struct timeval *tv, void* p)
{
  FILETIME ft;
  unsigned __int64 tmpres = 0;
  static int tzflag;
 
  if (NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);
 
    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;
 
    /*converting file time to unix epoch*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS; 
    tmpres /= 10;  /*convert into microseconds*/
    tv->tv_sec = (long)(tmpres / 1000000UL);
    tv->tv_usec = (long)(tmpres % 1000000UL);
  }
 
  return 0;
}

void timersub(struct timeval *a, struct timeval *b, struct timeval *res) {
  unsigned __int64 a_time = a->tv_sec*1000000UL + a->tv_usec;
  unsigned __int64 b_time = b->tv_sec*1000000UL + b->tv_usec;
  unsigned __int64 res_time = a_time - b_time;
  if (a_time < b_time) {
      res->tv_sec = 0;
      res->tv_usec = 0;
      return;
  }
  res->tv_sec  = (long)(res_time / 1000000UL);
  res->tv_usec = (long)(res_time % 1000000UL);
}

#endif
