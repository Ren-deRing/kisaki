// SPDX-License-Identifier: GPL-2.0-only
/*
 * kisaki Early Table Initializer
 *
 * Copyright (c) 2026 Ren-deRing
 *
 */

#include <asm/initvm.h>

#include <kisaki/arealloc.h>
#include <kisaki/panic.h>
#include <kisaki/printk.h>
#include <kisaki/string.h>

#include <stdbool.h>

#define DIV_ROUND_UP(n, d) (((n) / (d)) + !!((n) % (d)))
#define PAGE_SIZE 4 * 1024

static uintptr_t root_phys;

void initvm_enable_table() {
    asm volatile (
        "mov %0, %%cr3"
        :
        : "r" (root_phys)
        : "memory"
    );
}

void initvm_expand_table(uintptr_t new_limit) {
    uintptr_t limit = 2 * 1024 * 1024;
    bool enabled = false;

    uint64_t *pml4e = (uint64_t *)arealloc_alloc_below(PAGE_SIZE, PAGE_SIZE, limit);
    if (!pml4e) panic("initvm: can't allocate table: out of memory?");
    memset(pml4e, 0, PAGE_SIZE);
    root_phys = (uintptr_t)pml4e;

    for (size_t i = 0; i < 512; i++) {
        if (limit >= new_limit) break;

        uint64_t *pdpte = (uint64_t *)arealloc_alloc_below(PAGE_SIZE, PAGE_SIZE, limit);
        if (!pdpte) panic("initvm: can't allocate table: out of memory?");
        memset(pdpte, 0, PAGE_SIZE);
        pml4e[i] = ((uintptr_t)pdpte & PML4E_ADDR_MASK) | PTE_PRESENT | PTE_WRITABLE;
        
        for (size_t j = 0; j < 512; j++) {
            if (limit >= new_limit) break;

            uint64_t *pde = (uint64_t *)arealloc_alloc_below(PAGE_SIZE, PAGE_SIZE, limit);
            if (!pde) panic("initvm: can't allocate table: out of memory?");
            memset(pde, 0, PAGE_SIZE);
            pdpte[j] = ((uintptr_t)pde & PDPTE_ADDR_MASK) | PTE_PRESENT | PTE_WRITABLE;
            
            if (!enabled) limit = 0;

            for (size_t k = 0; k < 512; k++) {
                if (limit >= new_limit) break;
                pde[k] = (limit & PDE_ADDR_MASK) | PDE_PS | PTE_PRESENT | PTE_WRITABLE;
                limit += 2 * 1024 * 1024;
            }
            if (!enabled) { initvm_enable_table(); enabled = true; }
        }
    }

    initvm_enable_table(); // flush
}
