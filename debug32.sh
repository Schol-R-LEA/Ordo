qemu-system-i386 -d int,cpu_reset -no-reboot -s -S -boot order=a -fda "ordo.qcow2"  &
gdb -ex 'target remote localhost:1234' \
    -ex 'set archi i386:intel'         \
    -ex 'symbol-file obj/kernel.elf'   \
    -ex 'break *0xc0000000'
