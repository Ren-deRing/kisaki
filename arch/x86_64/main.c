#include "io.h"
#include "multiboot2.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "../../include/deps/stb_sprintf.h" // i'll fix it

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define COM1 0x3F8

size_t strlen(const char *s) {
    const char *p = s;
    while (*p) { p++; } // loop until reaches \0
    return (size_t)(p - s);
}

void *memcpy(void *dest, const void *src, size_t count) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    for (size_t i = 0; i < count; i++) {
        d[i] = s[i];
    }
    return dest;
}

void serial_putc(char c) {
    while ((inb(COM1 + 5) & 0x20) == 0);
    outb(0x3F8, c);
}

char* stb_callback(char const * buf, void * user, int len) {
    (void)user;

    for (int i = 0; i < len; i++) serial_putc(buf[i]);
    return (char *)buf;
}

void printk(const char* fmt, ...) {
    char buf[STB_SPRINTF_MIN];

    va_list args;
    va_start(args, fmt);

    stbsp_vsprintfcb(stb_callback, NULL, buf, fmt, args);

    va_end(args);
}

struct multiboot_tag {
    uint32_t type;
    uint32_t size;
};

const char * info_name_table[] = {
    [0]  = "END TAG",
    [1]  = "BOOT COMMAND LINE",
    [2]  = "BOOTLOADER NAME",
    [3]  = "MODULES",
    [4]  = "BASIC MEMORY INFO",
    [5]  = "BIOS BOOT DEVICE",
    [6]  = "MEMORY MAP",
    [7]  = "VBE INFO",
    [8]  = "FRAMEBUFFER INFO",
    [9]  = "ELF SYMBOLS",
    [10] = "AFM TABLE",
    [11] = "EFI 32-bit SYSTEM TALBE POINTER",
    [12] = "EFI 64-bit SYSTEM TABLE POINTER",
    [13] = "SMBIOS TABLES",
    [14] = "OLD RSDP",
    [15] = "NEW RSDP",
    [16] = "NETWORK INFO",
    [17] = "EFI MEMORY MAP",
    [18] = "EFI BS NOT TERMINATED",
    [19] = "EFI 32-bit IMAGE HANDLE POINTER",
    [20] = "EFI 64-bit IMAGE HANDLE POINTER",
    [21] = "IMAGE LOAD BASE PHYSICAL ADDRESS",
};

typedef enum {
    MMAP_FREE = 0,
    MMAP_RESERVED,
    MMAP_ACPI_RECLAIMABLE,
    MMAP_BAD_MEMORY,
} mtype_t;

const char * mmap_name_table[] = {
    [0]  = "MMAP_FREE",
    [1]  = "MMAP_RESERVED",
    [2]  = "MMAP_ACPI_RECLAIMABLE",
    [3]  = "MMAP_BAD_MEMORY",
};

mtype_t convert_memtype(uint64_t type) {
    switch (type) {
        case 1:  return MMAP_FREE;
        case 3:  return MMAP_ACPI_RECLAIMABLE;
        case 5:  return MMAP_BAD_MEMORY;
        default: return MMAP_RESERVED;
    }
}

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

void boot_main(uint32_t magic, uint32_t *info) {
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) for(;;);
    outb(COM1 + 1, 0x00); // no interrupt mode (polled mode)
    outb(COM1 + 3, 0x80); // DLAB on (speed setting mode)
    outb(COM1 + 0, 0x01); // baud rate 115200 (Low)
    outb(COM1 + 1, 0x00); // baud rate 115200 (High)
    outb(COM1 + 3, 0x03); // DLAB Off + 8 bits of text, no parity, 1 stop bit
    outb(COM1 + 2, 0xC7); // FIFO enable, clear FIFO, 14-byte threshold
    outb(COM1 + 4, 0x0B); // DTR, RTS, OUT2 set
    
    printk("Hello from kisaki\n\n");

    struct bootinfo binfo;

    struct multiboot_tag *tag = (struct multiboot_tag *)info;
    printk("Total Tag Size: %d, Reserved (0): %d\n\n", tag->type, tag->size);
    tag++;
    int i = 0;
    while (1) {
        printk("[%02d] [%s] size: %d\n", i, info_name_table[tag->type], tag->size);

        switch (tag->type) {
            case 2:
                binfo.loader_name_ptr = (uintptr_t)((uint8_t *)tag + 8);
            case 6:
                binfo.mmap_addr = (uintptr_t)((uint8_t *)tag + 16);
                binfo.mmap_entry_size = *((uint8_t *)tag + 8);
                binfo.mmap_count = (tag->size - 16) / binfo.mmap_entry_size;
            case 8:
                memcpy(&binfo.fb, ((uint8_t *)tag + 8), sizeof(binfo.fb));
            case 9:
                ;
            case 17:
                binfo.rsdp_addr = (uintptr_t)((uint8_t *)tag + 8);
        }

        if (tag->type == 0 & tag->size == 8) break;
        tag = (struct multiboot_tag *)(((uintptr_t)((uint8_t *)tag + tag->size) + 7) & ~7);
        i++;
    }

    printk("fb addr: 0x%llx\n", binfo.fb.addr);
    printk("fb wxh: %dx%d (pitch: %d)\n", binfo.fb.width, binfo.fb.height, binfo.fb.pitch);
    printk("fb bpp: %d\n", binfo.fb.bpp);

    // for (int i = 0; i < binfo.fb.height; i++) {
    //     for (int j = 0; j < binfo.fb.width; j++) {
    //         uint8_t *pixel = fb + i * binfo.fb.pitch + j * 3;
    //         pixel[0] = 0xCB;
    //         pixel[1] = 0xC0;
    //         pixel[2] = 0xFF;
    //     }
    // }
    // not mapped yet

    printk("rsdp addr: 0x%08lx\n", binfo.rsdp_addr);
    printk("bootloader: %s\n", binfo.loader_name_ptr);

    printk("memory map addr: 0x%08lx, count: %d\n", binfo.mmap_addr, binfo.mmap_count);

    for (int i = 0; i < (int)binfo.mmap_count; i++) {
        struct mmap_entry* map = (struct mmap_entry *)(binfo.mmap_addr + (i * binfo.mmap_entry_size));
        printk("[0x%08lx - 0x%08lx]: %s\n", map->base_addr, (map->base_addr + map->length), mmap_name_table[convert_memtype(map->type)]);
    }

    for (;;);
}
