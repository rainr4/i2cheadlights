#include "build.h"
#include <stdio.h>
#include <string.h>

void build_tm(tm* out_tm) {
    char s_month[5];
    int month, day, year;
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

    sscanf(__DATE__, "%s %d %d", s_month, &day, &year);
    month = (strstr(month_names, s_month) - month_names) / 3;

    out_tm->tm_mon = month;
    out_tm->tm_mday = day;
    out_tm->tm_year = year - 1900;
    out_tm->tm_isdst = -1;
    int d    = day   ; //Day     1-31
    int m    = month+1    ; //Month   1-12`
    int y    = year;
    // from https://stackoverflow.com/questions/6054016/c-program-to-find-day-of-week-given-date
    int weekday  = (d += m < 3 ? y-- : y - 2, 23*m/9 + d + 4 + y/4- y/100 + y/400)%7; 
    out_tm->tm_wday = weekday;
    int hr, min, sec;
    sscanf(__TIME__, "%2d:%2d:%2d", &hr, &min, &sec);
    out_tm->tm_hour = hr;
    out_tm->tm_min = min;
    out_tm->tm_sec = sec;
}
time_t build_time() {
    tm result;
    build_tm(&result);
    return mktime(&result);
}
