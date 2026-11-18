#pragma once

#include <stdint.h>

typedef enum {
    MMAP_FREE = 0,
    MMAP_RESERVED,
    MMAP_ACPI_RECLAIMABLE,
    MMAP_BAD_MEMORY,
} mtype_t;

struct framebuffer {
    uint64_t addr;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
    uint8_t type;
    uint8_t reserved;
};

struct mmap_entry {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
};

struct bootinfo {
    uintptr_t image_addr;
    uintptr_t rsdp_addr;
    struct framebuffer fb;

    uintptr_t loader_name_ptr;

    uintptr_t mmap_addr;
    uint32_t mmap_count;
    uint32_t mmap_entry_size;
};
