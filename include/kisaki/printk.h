#pragma once

typedef void (*putc_sink)(char c);

void printk(const char* fmt, ...);
void set_putc_sink(putc_sink new_putc);
