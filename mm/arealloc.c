// SPDX-License-Identifier: GPL-2.0-only
/*
 * kisaki Area-Based Boot-Only Allocator (arealloc)
 *
 * Copyright (c) 2026 Ren-deRing
 */

#include <kisaki/arealloc.h>
#include <kisaki/boot.h>
#include <kisaki/printk.h>
#include <kisaki/panic.h>

#include <stddef.h>
#include <stdint.h>

extern char kernel_start[];
extern char kernel_end[];

static struct arealloc_region regions[AREALLOC_MAX_REGIONS];
static size_t region_count;
static uintptr_t memory_max_addr;

static mtype_t convert_memtype(uint64_t type) {
    switch (type) {
        case 1:  return MMAP_FREE;
        case 3:  return MMAP_ACPI_RECLAIMABLE;
        case 5:  return MMAP_BAD_MEMORY;
        default: return MMAP_RESERVED;
    }
}

void arealloc_init() {
    uintptr_t start = (uintptr_t)kernel_start;
    uintptr_t end = (uintptr_t)kernel_end;

    int count = 0;
    for (int i = 0; i < (int)binfo.mmap_count; i++) {
        struct mmap_entry* map = (struct mmap_entry *)(binfo.mmap_addr + (i * binfo.mmap_entry_size));
        if (convert_memtype(map->type) == MMAP_FREE) {
            uintptr_t base = map->base_addr;
            uintptr_t length = map->length;

            if (base < AREALLOC_MIN_ADDR) continue;
            if (count >= AREALLOC_MAX_REGIONS)
                panic("arealloc: too many regions (max: %d)", AREALLOC_MAX_REGIONS);

            if (base <= start) {
                if (base + length >= end) {
                    // | base = kernel | | base + len |
                    if (base == start) {
                        base = end;
                    // | base | | kernel = base + len |
                    } else if (base + end == end) {
                        length = start - base;
                    // | base | | kernel | | base + len |
                    } else {
                        count++;
                        if (count >= AREALLOC_MAX_REGIONS)
                            panic("arealloc: too many regions (max: %d)", AREALLOC_MAX_REGIONS);
                        regions[count].address = end;
                        regions[count].size = (base + length) - end;
                        length = start - base;
                        printk("base < kernel < end\n");
                    }
                }
            }
            regions[count].address = base;
            regions[count].size = length;
            memory_max_addr = base + length;
            count++;
        }
    }
    region_count = count;

    // for (int i = 0; i < count; i++) {
    //     printk("[%d] 0x%016lx - 0x%016lx\n", i, regions[i].address, regions[i].address + regions[i].size);
    // }

    // printk("kernel start: 0x%08lx, end: 0x%08lx\n", kernel_start, kernel_end);
}

static void *alloc(size_t size, size_t align, uintptr_t limit) {
    if (!((align & (align - 1)) == 0)) return NULL;

    if (!align) align = 1;
    if (!limit) limit = UINTPTR_MAX;

    for (size_t i = 0; i < region_count; i++) {
        uintptr_t start = regions[i].address;
        if (start > UINTPTR_MAX - (align - 1)) return NULL;
        uintptr_t aligned = (start + align - 1) & ~(uintptr_t)(align - 1);
        size_t pad = aligned - start;

        if (regions[i].size < pad || regions[i].size - pad < size) continue;
        if (size > limit || aligned > limit - size) return NULL;

        size_t used = size + pad;
        if ((regions[i].size - used) == 0) {
            region_count--;
            for (size_t j = i; j < region_count; j++) {
                regions[j].address = regions[j+1].address;
                regions[j].size = regions[j+1].size;
            }
        } else {
            regions[i].address += used;
            regions[i].size -= used;
        }
        return (void *)aligned;
    }
    return NULL;
}

void *arealloc_alloc(size_t size, size_t align) {
    return alloc(size, align, 0);
}

void *arealloc_alloc_below(size_t size, size_t align, uintptr_t limit) {
    return alloc(size, align, limit);
}

uintptr_t arealloc_max_addr() {
    return memory_max_addr;
}
