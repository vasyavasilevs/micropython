/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2017 Paul Sokolovsky
 * Copyright (c) 2014-2023 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "helpers.h"

#include "shared/timeutils/timeutils.h"

#include "py/mphal.h"
#include "py/runtime.h"

typedef int time_t;

#define sleep_select select

#define MP_CLOCKS_PER_SEC CLOCKS_PER_SEC

#if defined(MP_CLOCKS_PER_SEC)
#define CLOCK_DIV (MP_CLOCKS_PER_SEC / MICROPY_FLOAT_CONST(1000.0))
#else
#error Unsupported clock() implementation
#endif

static mp_obj_t mp_time_sleep(mp_obj_t arg) {
    int seconds = mp_obj_get_int(arg);
    MP_THREAD_GIL_EXIT();
    sys_usleep(seconds * 1000000);
    MP_THREAD_GIL_ENTER();
    mp_handle_pending(true);
    return mp_const_none;
}

static mp_obj_t mp_time_time_get(void) {
    return mp_obj_new_int((mp_int_t)vsys_gettime());
}

static mp_obj_t mod_time_gmtime(size_t n_args, const mp_obj_t *args) {
    time_t t;
    if (n_args == 0) {
        // Linux sets RTC time to GMT, so does JOS.
        t = vsys_gettime();
    } else {
        #if MICROPY_PY_BUILTINS_FLOAT && MICROPY_FLOAT_IMPL == MICROPY_FLOAT_IMPL_DOUBLE
        mp_float_t val = mp_obj_get_float(args[0]);
        t = (time_t)MICROPY_FLOAT_C_FUN(trunc)(val);
        #else
        t = mp_obj_get_int(args[0]);
        #endif
    }

    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    mktime(t, &tm);

    mp_obj_t ret = mp_obj_new_tuple(9, NULL);

    mp_obj_tuple_t *tuple = MP_OBJ_TO_PTR(ret);

    int year = tm.tm_year + 1900;
    tuple->items[0] = MP_OBJ_NEW_SMALL_INT(year);

    tuple->items[1] = MP_OBJ_NEW_SMALL_INT(tm.tm_mon + 1);
    tuple->items[2] = MP_OBJ_NEW_SMALL_INT(tm.tm_mday);
    tuple->items[3] = MP_OBJ_NEW_SMALL_INT(tm.tm_hour);
    tuple->items[4] = MP_OBJ_NEW_SMALL_INT(tm.tm_min);
    tuple->items[5] = MP_OBJ_NEW_SMALL_INT(tm.tm_sec);

    int wday = tm.tm_wday - 1;
    if (wday < 0) {
        wday = 6;
    }
    tuple->items[6] = MP_OBJ_NEW_SMALL_INT(wday);

    int yday = timeutils_year_day(year, tm.tm_mon, tm.tm_mday);
    tuple->items[7] = MP_OBJ_NEW_SMALL_INT(yday);

    tuple->items[8] = MP_OBJ_NEW_SMALL_INT(tm.tm_isdst);

    return ret;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_time_gmtime_obj, 0, 1, mod_time_gmtime);

static mp_obj_t mod_time_mktime(mp_obj_t tuple) {
    size_t len;
    mp_obj_t *elem;
    mp_obj_get_array(tuple, &len, &elem);

    // localtime generates a tuple of len 8. CPython uses 9, so we accept both.
    if (len < 8 || len > 9) {
        mp_raise_TypeError(MP_ERROR_TEXT("mktime needs a tuple of length 8 or 9"));
    }

    struct tm time = {
        .tm_year = mp_obj_get_int(elem[0]) - 1900,
        .tm_mon = mp_obj_get_int(elem[1]) - 1,
        .tm_mday = mp_obj_get_int(elem[2]),
        .tm_hour = mp_obj_get_int(elem[3]),
        .tm_min = mp_obj_get_int(elem[4]),
        .tm_sec = mp_obj_get_int(elem[5]),
    };
    if (len == 9) {
        time.tm_isdst = mp_obj_get_int(elem[8]);
    } else {
        time.tm_isdst = -1; // auto-detect
    }
    time_t ret = timestamp(&time);
    if (ret == -1) {
        mp_raise_msg(&mp_type_OverflowError, MP_ERROR_TEXT("invalid mktime usage"));
    }
    return mp_obj_new_int(ret);
}
MP_DEFINE_CONST_FUN_OBJ_1(mod_time_mktime_obj, mod_time_mktime);

#define MICROPY_PY_TIME_EXTRA_GLOBALS \
    { MP_ROM_QSTR(MP_QSTR_gmtime), MP_ROM_PTR(&mod_time_gmtime_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_mktime), MP_ROM_PTR(&mod_time_mktime_obj) },
