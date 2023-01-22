#!/bin/bash

qemu-system-x86_64 -boot order=a -fda "ordo.qcow2" "$@" -m 512M

