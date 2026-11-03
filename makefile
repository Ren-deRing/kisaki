CFLAGS = -g -O2 -ffreestanding -fno-stack-protector -fno-PIC -fno-PIE -ffunction-sections -fdata-sections -Wall -Wextra -m64 -march=x86-64 -mno-red-zone

ALL:
	mkdir -p build/arch

	clang $(CFLAGS) -fno-integrated-as -c arch/x86_64/start.S -o build/arch/start.o

	ld.lld -T linker.ld -o build/kernel.elf --oformat=elf32-i386 build/arch/start.o -no-pie

	mkdir -p build/root/boot
	mkdir -p build/root/boot/grub

	cp build/kernel.elf build/root/boot
	cp grub.cfg build/root/boot/grub/grub.cfg

	grub2-mkrescue -o boot.iso build/root
	qemu-system-x86_64 -bios /usr/share/OVMF/OVMF_CODE.fd -cpu host -accel kvm -cdrom boot.iso