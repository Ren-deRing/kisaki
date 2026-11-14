#include "io.h"
#include "multiboot2.h"

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

void print(const char* fmt) {
    for (int i = 0; i < strlen(fmt); i++) {
        serial_putc(fmt[i]);
    }
}

void boot_main(uint32_t magic, uint32_t *info) {
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {};
    outb(COM1 + 1, 0x00); // no interrupt mode (polled mode)
    outb(COM1 + 3, 0x80); // DLAB on (speed setting mode)
    outb(COM1 + 0, 0x01); // baud rate 115200 (Low)
    outb(COM1 + 1, 0x00); // baud rate 115200 (High)
    outb(COM1 + 3, 0x03); // DLAB Off + 8 bits of text, no parity, 1 stop bit
    outb(COM1 + 2, 0xC7); // FIFO enable, clear FIFO, 14-byte threshold
    outb(COM1 + 4, 0x0B); // DTR, RTS, OUT2 set

    print("hello!\n");

    for (;;);
}
