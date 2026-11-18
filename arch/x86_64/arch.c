#include <asm/arch.h>

void arch_pause() {
    asm volatile ("pause");
}

void arch_halt() {
    asm volatile ("hlt");
}
