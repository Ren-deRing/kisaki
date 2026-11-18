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

static struct arealloc_region regions[AREALLOC_MAX_REGIONS];
static size_t region_count;

static mtype_t convert_memtype(uint64_t type) {
    switch (type) {
        case 1:  return MMAP_FREE;
        case 3:  return MMAP_ACPI_RECLAIMABLE;
        case 5:  return MMAP_BAD_MEMORY;
        default: return MMAP_RESERVED;
    }
}

void arealloc_init() {
    int count = 0;
    for (int i = 0; i < (int)binfo.mmap_count; i++) {
        struct mmap_entry* map = (struct mmap_entry *)(binfo.mmap_addr + (i * binfo.mmap_entry_size));
        if (convert_memtype(map->type) == MMAP_FREE) {
            if (map->base_addr < AREALLOC_MIN_ADDR) continue;
            if (count >= AREALLOC_MAX_REGIONS)
                panic("arealloc: too many regions (max: %d)", AREALLOC_MAX_REGIONS);
            regions[count].address = map->base_addr;
            regions[count].size = map->length;
            count++;
        }
    }
    region_count = count;
}

static void *alloc(size_t size, size_t align, uintptr_t limit) { // TODO: see where kernel image loaded first.
    if (size > SIZE_MAX - align) return NULL;
    if (align > 16) return NULL;
    if (!((align & (align - 1)) == 0)) return NULL;

    if (!limit) limit = UINTPTR_MAX;
    size += align;

    for (size_t i = 0; i < region_count; i++) {
        if (regions[i].size >= size) {
            if (size > UINTPTR_MAX - regions[i].address) return NULL;
            if (regions[i].address + size > limit) return NULL;

            uintptr_t addr = (uintptr_t)regions[i].address;

            if ((regions[i].size - size) == 0) {
                region_count--;
                for (size_t j = i; j < region_count; j++) {
                    regions[j].address = regions[j+1].address;
                    regions[j].size = regions[j+1].size;
                }
            } else {
                regions[i].address += size;
                regions[i].size -= size;
            }

            for (size_t i = 0; i < region_count; i++) {
                printk("[%d] %016lx\n", i, regions[i].address);
            }

            if (align != 0)
                return (void *)((addr + align - 1) & ~(align - 1));
            return (void *)addr;
        }
    }
    return NULL;
}

void *arealloc_alloc(size_t size, size_t align) {
    return alloc(size, align, 0);
}

void *arealloc_alloc_below(size_t size, size_t align, uintptr_t limit) {
    return alloc(size, align, limit);
}
