#!/bin/bash

# first arg is builddir, second is out file, and third is build system arg

grub-mkrescue -d /usr/lib/grub/i386-pc -o $1/kernel.iso $1/iso

[[ $3 = run ]] && qemu-system-x86_64 -m 5120 -cdrom $1/kernel.iso

if [[ $3 = debug ]]
then
	#termite qemu-system-x86_64 -m 5120 -debugcon stdio -s -S $1/kernel.iso & $TERM -e "gdb -x debug.gdb"
	qemu-system-x86_64 -m 5120 -debugcon stdio -s -S $1/kernel.iso & $TERM -e "/bin/gdb" "-x" "debug.gdb"
fi

if [[ $3 = bochs ]]
then
	$TERM -e bochs -f bochsrc
fi
