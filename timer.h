#ifndef TIMER_H
#define TIMER_H

#include "utils.h"

#include <stdlib.h>

#ifdef WIN32
#include "gettimeofday.h"
#else
#include <sys/time.h>
#endif

class Timer {
private:
    struct timeval _start;
    struct timeval _last;

public:
    Timer();

    void     start();
    uint32_t check();
    uint32_t total();

};

#endif // TIMER_H
