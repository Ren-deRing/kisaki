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
    for (int i = 0; i < len; i++) serial_putc(buf[i]);
    return (char *)buf;
}

void kprintf(const char* fmt, ...) {
    char buf[STB_SPRINTF_MIN];

    va_list args;
    va_start(args, fmt);

    stbsp_vsprintfcb(stb_callback, NULL, buf, fmt, args);

    va_end(args);
}

void boot_main(uint32_t magic, uint32_t *info) {
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) for(;;);
    outb(COM1 + 1, 0x00); // no interrupt mode (polled mode)
    outb(COM1 + 3, 0x80); // DLAB on (speed setting mode)
    outb(COM1 + 0, 0x01); // baud rate 115200 (Low)
    outb(COM1 + 1, 0x00); // baud rate 115200 (High)
    outb(COM1 + 3, 0x03); // DLAB Off + 8 bits of text, no parity, 1 stop bit
    outb(COM1 + 2, 0xC7); // FIFO enable, clear FIFO, 14-byte threshold
    outb(COM1 + 4, 0x0B); // DTR, RTS, OUT2 set

    kprintf("hello from long mode C!\n");
    kprintf("and stb_sprintf! maigc: %lx\n", magic);

    for (;;);
}
