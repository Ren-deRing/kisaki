#include <asm/arch.h>
#include <kisaki/printk.h>

void init_kernel() {
    printk("kisaki generic init entry!\n");

    for(;;) arch_halt();
}
