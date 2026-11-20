ARCH = x86_64

CFLAGS = -g -O2 -ffreestanding -fno-stack-protector -fno-PIC -fno-PIE -ffunction-sections -fdata-sections \
	-Wall -Wextra -m64 -march=x86-64 -mno-red-zone \
	-Iinclude \
	-Iarch/$(ARCH)

all:
	mkdir -p build/
	mkdir -p build/arch/$(ARCH)
	mkdir -p build/arch/$(ARCH)/kernel
	mkdir -p build/arch/$(ARCH)/boot

	clang $(CFLAGS) -fno-integrated-as -c arch/x86_64/boot/start.S -o build/arch/x86_64/boot/start.o
	clang $(CFLAGS) -c arch/x86_64/boot/main.c -o build/arch/x86_64/boot/main.o

	clang $(CFLAGS) -c arch/x86_64/kernel/arch.c -o build/arch/x86_64/kernel/arch.o
	clang $(CFLAGS) -c arch/x86_64/kernel/initvm.c -o build/arch/x86_64/kernel/initvm.o

	mkdir -p build/kernel
	mkdir -p build/kernel/misc

	clang $(CFLAGS) -c kernel/misc/printk.c -o build/kernel/misc/printk.o
	clang $(CFLAGS) -c kernel/misc/string.c -o build/kernel/misc/string.o
	clang $(CFLAGS) -c kernel/misc/panic.c  -o build/kernel/misc/panic.o

	mkdir -p build/mm

	clang $(CFLAGS) -c mm/arealloc.c -o build/mm/arealloc.o

	mkdir -p build/init

	clang $(CFLAGS) -c init/main.c -o build/init/main.o

	ld.lld -T linker.ld -o build/kernel.elf --oformat=elf64-x86_64 \
		build/arch/x86_64/boot/start.o build/arch/x86_64/boot/main.o \
		build/arch/x86_64/kernel/arch.o build/arch/x86_64/kernel/initvm.o \
		build/kernel/misc/printk.o build/kernel/misc/string.o build/kernel/misc/panic.o \
		build/init/main.o \
		build/mm/arealloc.o -no-pie

	mkdir -p build/root/boot
	mkdir -p build/root/boot/grub

	cp build/kernel.elf build/root/boot
	cp grub.cfg build/root/boot/grub/grub.cfg

	grub2-mkrescue -o boot.iso build/root

run: all
	qemu-system-x86_64 -bios /usr/share/OVMF/OVMF_CODE.fd -cpu host -accel kvm -cdrom boot.iso -serial stdio -d int -m 8G

clean:
	rm -rf build