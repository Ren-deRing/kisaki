#pragma once

#include <stdarg.h>

typedef void (*putc_sink)(char c);

void printk(const char* fmt, ...);
void vprintk(const char *fmt, va_list args);
void set_putc_sink(putc_sink new_putc);
