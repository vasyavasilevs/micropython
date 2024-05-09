#include "helpers.h"

#include "lib/oofatfs/ff.h"

DWORD get_fattime(void) {
    int now = vsys_gettime();
    struct tm tnow;

    mktime(now, &tnow);
    return ((1900 + tnow.tm_year - 1980) << 25)
           | ((tnow.tm_mon + 1) << 21)
           | (tnow.tm_mday << 16)
           | (tnow.tm_hour << 11)
           | (tnow.tm_min << 5)
           | (tnow.tm_sec / 2);
}
