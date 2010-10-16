#include "timer.h"

Timer::Timer() {
    start();
}

void Timer::start() {
    gettimeofday(&_start, NULL);
    _last = _start;
}

uint32_t Timer::check() {
    struct timeval cur;
    gettimeofday(&cur, NULL);
    timersub(&cur, &_last, &_last);
    uint32_t ret = _last.tv_sec * 1000 + (_last.tv_usec / 1000);
    _last = cur;
    return ret;
}

uint32_t Timer::total() {
    struct timeval cur;
    gettimeofday(&cur, NULL);
    timersub(&cur, &_start, &cur);
    return cur.tv_sec * 1000 + (cur.tv_usec / 1000);
}
