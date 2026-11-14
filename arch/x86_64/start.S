#define ASM_FILE 1
#include "multiboot2.h"

.code32

.section .multiboot
.balign 8

multiboot_header:
    .long MULTIBOOT2_HEADER_MAGIC
    .long 0 # protected mode i386
    .long (multiboot_header_end - multiboot_header)
    .long -(MULTIBOOT2_HEADER_MAGIC + 0 + (multiboot_header_end - multiboot_header))

    .short MULTIBOOT_HEADER_TAG_END
    .short 0
    .long 8
multiboot_header_end:

.text
.equ SIZEOF_PAGE_TABLE, 4096

.equ PT_ADDR_MASK, 0xfffff000
.equ PT_PRESENT, 1
.equ PT_WRITABLE, 2

.equ ENTRIES_PER_PT, 512
.equ SIZEOF_PT_ENTRY, 8
.equ PAGE_SIZE, 0x1000

.equ CR4_PAE_ENABLE, 1 << 5
.equ EFER_MSR, 0xC0000080
.equ EFER_LM_ENABLE, 1 << 8
.equ CR0_PM_ENABLE, 1 << 0
.equ CR0_PG_ENABLE, 1 << 31

.globl _start
_start:
    mov %eax, magic
    mov %ebx, info

    # Stack pointer
    movl $(stack + 8196), %esp

    # Reset EFLAGS
    pushl $0
    popf

    lgdt (gdtr)

    mov $boot_pml4, %edi # memset target
    mov %edi, %cr3       # &(head of page tables)

    xor %eax, %eax               # memset value (0)
    mov $SIZEOF_PAGE_TABLE, %ecx # 'll repeat 4096 times
    rep stosl                    # fill 4B zero to edi for ecx times (clearing 16KB for pages)
    mov %cr3, %edi               # back to PML4T ADDR

    mov $boot_pdpt, %eax
    andl $PT_ADDR_MASK, %eax
    orl $(PT_PRESENT | PT_WRITABLE), %eax
    movl %eax, (%edi) # write PML4T (PDPT's phys addr, flags)

    mov $boot_pdpt, %edi
    mov $boot_pdt, %eax
    andl $PT_ADDR_MASK, %eax
    orl $(PT_PRESENT | PT_WRITABLE), %eax
    movl %eax, (%edi) # write PDPT  (same but PDT's phys addr)

    mov $boot_pdt, %edi
    mov $boot_pt, %eax
    andl $PT_ADDR_MASK, %eax
    orl $(PT_PRESENT | PT_WRITABLE), %eax
    movl %eax, (%edi) # write PDT   (too)

    mov $boot_pt, %edi                    # page table's addr (start addr: PT_ADDR)
    mov $(PT_PRESENT | PT_WRITABLE), %ebx # page table content (0x00000003)
    mov $ENTRIES_PER_PT, %ecx             # full page table (2MiB)

entry_set:
    movl %ebx, (%edi)          # write entry
    add $PAGE_SIZE, %ebx       # next phys addr (0x00000003 + 0x1000 = 0x00001003)
    add $SIZEOF_PT_ENTRY, %edi # move to next page table entry
    loop entry_set             # loop, ecx--

long_setup:
    mov %cr4, %eax            # read cr4
    or $CR4_PAE_ENABLE, %eax  # enable PAE
    mov %eax, %cr4            # write back

    mov $EFER_MSR, %ecx       # extended feature enable register's msr
    rdmsr                     # read
    or $EFER_LM_ENABLE, %eax  # enable long mode
    wrmsr                     # write back

    mov %cr0, %eax           # read cr0
    or $CR0_PG_ENABLE, %eax  # set PG bit (paging enable)
    mov %eax, %cr0           # write back 

    ljmp $0x18, $_start64 # Ring 0, Use GDT, GDT Idx 3

.code64
_start64:
    xor %ax, %ax # long mode doesn't use segmentation
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov %ax, %fs
    mov %ax, %gs

    mov $0x1FFFF0, %rsp
    mov $0x1FFFF0, %rbp

    mov info, %esi # boot *infomation
    mov magic, %edi # magic

    .extern boot_main
    call boot_main # Call C main

    cli

loop:
    hlt
    jmp loop

gdt:
    .quad 0

    .long 0x0000FFFF # Base: 0000 Limit: FFFF (4GB)
    .long 0x00CF9A00 # (1(G)1(DB)0(L)0(X) = C) (1(P)00(DPL)1(S)1(E)0(C)1(RW)0(A) = 9A)

    .long 0x0000FFFF
    .long 0x00CF9200 # (1(P)00(DPL)1(S)0(E)0(D)1(RW)0(A) = 92)

    # long mode
    .long 0x00000000
    .long 0x00209A00 # (0(G)0(DB)1(L)0(X) = 2) (1(P)00(DPL)1(S)1(E)0(C)1(RW)0(A) = 9A)
gdt_end:

gdtr:
    .word (gdt_end - gdt) - 1  # GDT is always bigger than 1 Byte.
    .long gdt                  # GDT's addr

.bss

.balign 8

magic: .long 0
info:  .long 0

.balign 4096

boot_pml4: .zero 4096
boot_pdpt: .zero 4096
boot_pdt:  .zero 4096
boot_pt:   .zero 4096

.zero 8196
stack: