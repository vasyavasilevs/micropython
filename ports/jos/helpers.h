#ifndef MICROPY_INCLUDED_JOS_HELPERS_H
#define MICROPY_INCLUDED_JOS_HELPERS_H

#undef RAND_MAX
#undef va_start
#undef va_copy
#undef va_arg
#undef va_end
#undef fprintf
#undef vfprintf
#undef exit
#undef off_t

#include <inc/assert.h>
#include <inc/lib.h>
#include <inc/random.h>
#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/time.h>
#include <inc/types.h>

ssize_t fdputs(const char *s, int fd);
char *fdgets(char *s, int n, int fd);

ssize_t getrandom(void *buffer, size_t length);

// Math lib implementations.
double round(double);
double modf(double x, double *iptr);
double trunc(double x);
double fabs(double);

#endif // MICROPY_INCLUDED_JOS_HELPERS_H
