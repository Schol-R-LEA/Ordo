#!/bin/bash

qemu-system-x86_64 -boot order=a -fda "ordo.qcow2" "$@" -m 128M -d int -accel tcg