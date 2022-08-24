qemu-system-i386 -d int,cpu_reset -no-reboot -s -S -boot order=a -fda "ordo.qcow2"  &
gdb -ex 'target remote localhost:1234' \
    -ex 'archi i386:intel'             \
    -ex 'break *0xc0000000'
