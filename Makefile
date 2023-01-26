ASM = nasm -w+all
COPY = dd
FORMAT = mkfs.msdos -F 12 -n "ORDO"
REIMAGE=qemu-img
SYS_INSTALL = ~/Deployments/ms-sys-2.5.3/bin/ms-sys --fat12
BOOTPATH=Verbum/src/PC-x86/nasm/fat12
BOOT = verbum
STAGE_TWO = stagetwo
DISKTARGET = boot.img
DISKSIZE = 1440
CC=i686-elf-gcc
LD=i686-elf-ld
LINK_SCRIPT=linker.ld
CFLAGS=-Wall -Werror -Wpedantic -std=c2x -ffreestanding -ggdb
C_SRC=src
C_INCLUDES=-I $(C_SRC)/include
OBJPATH=obj
KERNEL=kernel
KSTART=kstart
GDT=gdt
GDT_SET=gdt_set
IDT=idt
TERMINAL=terminal
MEM=mem
PAGING=paging
ACPI=acpi


install: boot stage2 link
	mkdir -p obj
	$(COPY) if=/dev/zero of=$(OBJPATH)/$(DISKTARGET) count=$(DISKSIZE) bs=1k
	$(FORMAT) $(OBJPATH)/$(DISKTARGET)
	$(COPY) if=$(OBJPATH)/$(BOOT).bin of=$(OBJPATH)/$(DISKTARGET) count=1 conv=notrunc
	mkdir -p temp
	sudo mount $(OBJPATH)/$(DISKTARGET) temp
	sudo cp $(OBJPATH)/$(STAGE_TWO).bin temp/STAGETWO.SYS
	sudo cp $(OBJPATH)/$(KERNEL).elf temp/KERNEL.SYS
	sudo umount temp
	rmdir temp
	$(REIMAGE) convert -f raw -O qcow2 $(OBJPATH)/$(DISKTARGET) ordo.qcow2

link: kstart kernel terminal mem idt acpi paging gdt
	$(LD) -T $(LINK_SCRIPT)

kernel: terminal paging mem idt gdt acpi
	$(CC) $(CFLAGS) $(C_INCLUDES) -c $(C_SRC)/$(KERNEL).c -o $(OBJPATH)/$(KERNEL).o

acpi: terminal
	$(CC) $(CFLAGS) $(C_INCLUDES) -c $(C_SRC)/$(ACPI).c -o $(OBJPATH)/$(ACPI).o

idt: terminal
	$(CC) $(CFLAGS) -mgeneral-regs-only $(C_INCLUDES) -c $(C_SRC)/$(IDT).c -o $(OBJPATH)/$(IDT).o

gdt:
	$(ASM) -f elf32 $(C_SRC)/$(GDT_SET).asm -o $(OBJPATH)/$(GDT_SET).o -l $(OBJPATH)/$(GDT_SET).lst
	$(CC) $(CFLAGS) $(C_INCLUDES) -c $(C_SRC)/$(GDT).c -o $(OBJPATH)/$(GDT).o

paging: terminal mem
	$(CC) $(CFLAGS) $(C_INCLUDES) -c $(C_SRC)/$(PAGING).c -o $(OBJPATH)/$(PAGING).o

mem: terminal
	$(CC) $(CFLAGS) $(C_INCLUDES) -c $(C_SRC)/$(MEM).c -o $(OBJPATH)/$(MEM).o

terminal:
	$(CC) $(CFLAGS) $(C_INCLUDES) -c $(C_SRC)/$(TERMINAL).c -o $(OBJPATH)/$(TERMINAL).o

kstart:
	$(ASM) -f elf32 $(C_SRC)/$(KSTART).asm -o $(OBJPATH)/$(KSTART).o -l $(OBJPATH)/$(KSTART).lst

boot:
	$(ASM) -f bin -I$(BOOTPATH) $(BOOTPATH)/$(BOOT).asm -o $(OBJPATH)/$(BOOT).bin -l $(OBJPATH)/$(BOOT).lst

stage2:
	$(ASM) -f bin -I$(BOOTPATH) $(BOOTPATH)/$(STAGE_TWO).asm -o $(OBJPATH)/$(STAGE_TWO).bin -l $(OBJPATH)/$(STAGE_TWO).lst
