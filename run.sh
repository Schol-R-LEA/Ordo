#!/bin/bash

qemu-system-x86_64 -boot order=a -fda "ordo.qcow2" \
-m 128M                                            \
-enable-kvm                                        \
-vga virtio -display sdl,gl=on                     \
-serial stdio                                      \
-smp 1                                             \
-usb

