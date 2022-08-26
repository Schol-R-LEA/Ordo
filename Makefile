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
CFLAGS=-Wall -Werror -Wpedantic -std=c11 -ffreestanding
C_SRC=src
C_INCLUDES=-I $(C_SRC)
OBJPATH=obj
KERNEL=kernel
KSTART=kstart
TERMINAL=terminal
MEM_MAP=mem_map

install: boot stage2 link
	$(COPY) if=/dev/zero of=$(OBJPATH)/$(DISKTARGET) count=$(DISKSIZE) bs=1k
	$(FORMAT) $(OBJPATH)/$(DISKTARGET)
	$(COPY) if=$(OBJPATH)/$(BOOT).bin of=$(OBJPATH)/$(DISKTARGET) count=1 conv=notrunc
	mkdir temp
	sudo mount $(OBJPATH)/$(DISKTARGET) temp
	sudo cp $(OBJPATH)/$(STAGE_TWO).bin temp/STAGETWO.SYS
	sudo cp $(OBJPATH)/$(KERNEL).elf temp/KERNEL.SYS
	sudo umount temp
	rmdir temp
	$(REIMAGE) convert -f raw -O qcow2 $(OBJPATH)/$(DISKTARGET) ordo.qcow2

link: kstart kernel terminal mem_map
	$(LD) -T $(LINK_SCRIPT) $(OBJPATH)/$(KERNEL).o $(OBJPATH)/$(TERMINAL).o $(OBJPATH)/$(MEM_MAP).o -o $(OBJPATH)/$(KERNEL).elf

kernel: terminal
	$(CC) $(CFLAGS) $(C_INCLUDES) -c $(C_SRC)/$(KERNEL).c -o $(OBJPATH)/$(KERNEL).o

mem_map:
	$(CC) $(CFLAGS) $(C_INCLUDES) -c $(C_SRC)/$(MEM_MAP).c -o $(OBJPATH)/$(MEM_MAP).o

terminal:
	$(CC) $(CFLAGS) $(C_INCLUDES) -c $(C_SRC)/$(TERMINAL).c -o $(OBJPATH)/$(TERMINAL).o

kstart:
	$(ASM) -f elf32 $(C_SRC)/$(KSTART).asm -o $(OBJPATH)/$(KSTART).o -l $(OBJPATH)/$(KSTART).lst

boot:
	$(ASM) -f bin -I$(BOOTPATH) $(BOOTPATH)/$(BOOT).asm -o $(OBJPATH)/$(BOOT).bin -l $(OBJPATH)/$(BOOT).lst

stage2:
	$(ASM) -f bin -I$(BOOTPATH) $(BOOTPATH)/$(STAGE_TWO).asm -o $(OBJPATH)/$(STAGE_TWO).bin -l $(OBJPATH)/$(STAGE_TWO).lst
