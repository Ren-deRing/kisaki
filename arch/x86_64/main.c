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

    struct multiboot_tag *tag = (struct multiboot_tag *)info;
    printk("Total Tag Size: %d, Reserved (0): %d\n\n", tag->type, tag->size);
    tag++;
    int i = 0;
    while (1) {
        printk("[%02d] [%s] size: %d\n", i, info_name_table[tag->type], tag->size);
        if (tag->type == 0 & tag->size == 8) break;
        tag = (struct multiboot_tag *)(((uintptr_t)((uint8_t *)tag + tag->size) + 7) & ~7);
        i++;
    }

    for (;;);
}
