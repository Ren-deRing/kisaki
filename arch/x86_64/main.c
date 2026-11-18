#include "io.h"
#include "multiboot2.h"

#include <kisaki/boot.h>
#include <kisaki/string.h>
#include <kisaki/printk.h>

#define COM1 0x3F8

void serial_putc(char c) {
    while ((inb(COM1 + 5) & 0x20) == 0);
    outb(0x3F8, c);
}

struct multiboot_tag {
    uint32_t type;
    uint32_t size;
};

mtype_t convert_memtype(uint64_t type) {
    switch (type) {
        case 1:  return MMAP_FREE;
        case 3:  return MMAP_ACPI_RECLAIMABLE;
        case 5:  return MMAP_BAD_MEMORY;
        default: return MMAP_RESERVED;
    }
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
    
    set_putc_sink(serial_putc);

    printk("Hello from kisaki\n\n");

    struct bootinfo binfo;

    struct multiboot_tag *tag = (struct multiboot_tag *)info;
    tag++;
    while (1) {
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
    }

    // printk("fb addr: 0x%llx\n", binfo.fb.addr);
    // printk("fb wxh: %dx%d (pitch: %d)\n", binfo.fb.width, binfo.fb.height, binfo.fb.pitch);
    // printk("fb bpp: %d\n", binfo.fb.bpp);

    // for (int i = 0; i < binfo.fb.height; i++) {
    //     for (int j = 0; j < binfo.fb.width; j++) {
    //         uint8_t *pixel = fb + i * binfo.fb.pitch + j * 3;
    //         pixel[0] = 0xCB;
    //         pixel[1] = 0xC0;
    //         pixel[2] = 0xFF;
    //     }
    // }
    // not mapped yet

    // printk("rsdp addr: 0x%08lx\n", binfo.rsdp_addr);
    // printk("bootloader: %s\n", binfo.loader_name_ptr);

    // printk("memory map addr: 0x%08lx, count: %d\n", binfo.mmap_addr, binfo.mmap_count);

    // for (int i = 0; i < (int)binfo.mmap_count; i++) {
    //     struct mmap_entry* map = (struct mmap_entry *)(binfo.mmap_addr + (i * binfo.mmap_entry_size));
    //     printk("[0x%016lx - 0x%016lx]: %s\n", map->base_addr, (map->base_addr + map->length), mmap_name_table[convert_memtype(map->type)]);
    // }

    extern void init_kernel();
    init_kernel();
}
