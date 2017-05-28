QEMU := qemu-system-i386 -serial mon:stdio -d guest_errors

CFLAGS := -std=c99 -m32 -O1 -ggdb3 -gdwarf-4 -Wall -fasm -nostdinc
CFLAGS += -fno-pic -fno-inline -fno-omit-frame-pointer -ffreestanding
ASFLAGS := $(CFLAGS)

SOURCES := $(wildcard *.c)
OBJECTS := $(SOURCES:%.c=%.o)

kernel: entry.o swtch.o $(OBJECTS)
	ld -m elf_i386 -Ttext 0x100000 -o $@ $^
	objdump -S $@ >$@.asm
# Verificar que realmente hemos producido una imagen Multiboot v1.
	grub-file --is-x86-multiboot $@

qemu: kernel
	$(QEMU) -kernel $<

qemu-gdb: kernel
	$(QEMU) -kernel $< -nographic -S -gdb tcp:127.0.0.1:7508

gdb:
	gdb -q -s kernel -ex 'target remote 127.0.0.1:7508' -n -x .gdbinit

clean:
	rm -f kernel kernel.asm *.o core

.PHONY: clean qemu qemu-gdb gdb
