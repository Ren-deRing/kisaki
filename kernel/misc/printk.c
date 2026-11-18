// SPDX-License-Identifier: GPL-2.0-only
/*
 * kisaki Kernel Logger
 *
 * Copyright (c) 2026 Ren-deRing
 *
 * Uses stb_sprintf:
 * Copyright (c) 2017 Sean Barrett
 */

#include <kisaki/printk.h>

#define STB_SPRINTF_IMPLEMENTATION
#include <deps/stb_sprintf.h>

putc_sink kernel_putc;

void set_putc_sink(putc_sink new_putc) {
    kernel_putc = new_putc;
}

char* stb_callback(char const * buf, void * user, int len) {
    (void)user;

    for (int i = 0; i < len; i++) {
        if (kernel_putc) kernel_putc(buf[i]);
    }
    return (char *)buf;
}

void vprintk(const char *fmt, va_list args) {
    char buf[STB_SPRINTF_MIN];

    stbsp_vsprintfcb(stb_callback, NULL, buf, fmt, args);
}

void printk(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vprintk(fmt, args);

    va_end(args);
}
