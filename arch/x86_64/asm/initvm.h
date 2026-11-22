// SPDX-License-Identifier: GPL-2.0-only
#pragma once

#include <stddef.h>
#include <stdint.h>

#define PML4E_ADDR_MASK 0x000FFFFFFFFFF000ULL
#define PDPTE_ADDR_MASK 0x000FFFFFFFFFF000ULL
#define PDE_ADDR_MASK 0x000FFFFFFFE00000ULL

#define PTE_PRESENT  (1ULL << 0)
#define PTE_WRITABLE (1ULL << 1)

#define PDE_PS       (1ULL << 7)

void initvm_expand_table(uintptr_t new_limit);
