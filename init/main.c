// SPDX-License-Identifier: GPL-2.0-only
/*
 * kisaki Kernel Initializer
 *
 * Copyright (c) 2026 Ren-deRing
 *
 */

#include <asm/arch.h>
#include <asm/initvm.h>

#include <kisaki/boot.h>
#include <kisaki/printk.h>
#include <kisaki/arealloc.h>
struct bootinfo binfo;

void init_kernel() {
    printk("kisaki generic init entry!\n");

    arealloc_init();
    initvm_expand_table(arealloc_max_addr());

    for(;;) arch_halt();
}
