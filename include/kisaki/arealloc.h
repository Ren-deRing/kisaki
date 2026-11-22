// SPDX-License-Identifier: GPL-2.0-only
#pragma once

#include <stddef.h>
#include <stdint.h>

#define AREALLOC_MAX_REGIONS 128
#define AREALLOC_MIN_ADDR 0x100000

struct arealloc_region {
    uintptr_t address;
    size_t    size; 
};

void arealloc_init();
void *arealloc_alloc(size_t size, size_t align);
void *arealloc_alloc_below(size_t size, size_t align, uintptr_t limit);
uintptr_t arealloc_max_addr();
