#ifndef BUILD_H
#define BUILD_H
#include <time.h>
time_t build_time();
void build_tm(tm* out_tm);
#endif // BUILD_H