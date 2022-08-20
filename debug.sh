qemu-system-x86_64 -s -S -boot order=a -fda "ordo.qcow2" &
gdb -ex 'target remote localhost:1234' \
    -ex 'set architecture i8086' \
    -ex 'break *0xACA8' 