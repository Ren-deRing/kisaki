// SPDX-License-Identifier: GPL-2.0-only
/*
 * kisaki x86_64 Architecture Primitives
 *
 * Copyright (c) 2026 Ren-deRing
 *
 */

#include <asm/arch.h>

void arch_pause() {
    asm volatile ("pause");
}

void arch_halt() {
    asm volatile ("hlt");
}

void arch_panic() {
    for (;;) {
        asm volatile ("cli");
        asm volatile ("hlt");
    }
}
