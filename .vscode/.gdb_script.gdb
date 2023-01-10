    symbol-file bin/kernel.elf
    set disassembly-flavor intel
    target remote | qemu-system-i386 -s -S -gdb stdio -m 32 -fda boot.qcow2