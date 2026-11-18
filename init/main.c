// SPDX-License-Identifier: GPL-2.0-only
/*
 * kisaki Kernel Initializer
 *
 * Copyright (c) 2026 Ren-deRing
 *
 */

#include <asm/arch.h>

#include <kisaki/boot.h>
#include <kisaki/printk.h>
#include <kisaki/arealloc.h>

struct bootinfo binfo;

void init_kernel() {
    printk("kisaki generic init entry!\n");

    arealloc_init();
    void* alloc1 = arealloc_alloc_below(4 * 1024, 0, 2 * 1024 * 1024);
    printk("allocated: 0x%lx\n", (uintptr_t)alloc1);

    for(;;) arch_halt();
}
