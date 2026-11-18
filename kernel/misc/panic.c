// SPDX-License-Identifier: GPL-2.0-only

#include <asm/arch.h>
#include <kisaki/printk.h>

void panic(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    printk("panic: ");
    vprintk(fmt, args);
    printk("\n");

    va_end(args);

    arch_panic();
}
