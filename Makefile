# Compiler tools (requires a cross-compiler or a 32-bit Linux environment)
# CC = i686-elf-gcc
# AS = i686-elf-as
# If you don't have i686-elf-gcc, you can try standard gcc with -m32
CC = gcc -m32
AS = as --32

CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib -lgcc

OBJS = src/boot.o src/kernel.o src/graphics.o src/idt.o src/isr.o src/pic.o src/mouse.o

all: MossOS.iso

src/%.o: src/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

src/%.o: src/%.s
	$(AS) $< -o $@

MossOS.bin: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS)

MossOS.iso: MossOS.bin
	mkdir -p isodir/boot/grub
	cp MossOS.bin isodir/boot/MossOS.bin
	cp grub/grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o MossOS.iso isodir

clean:
	rm -f src/*.o MossOS.bin MossOS.iso
	rm -rf isodir

.PHONY: all clean
