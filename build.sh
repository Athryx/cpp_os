#!/bin/bash

ARCH=x64

PROJECT=$(dirname $0)
cd $PROJECT

BUILDDIR=abuild

SUBDIRS="initfs"

SRC=src
INCL=include
ARC=$SRC/arch/$ARCH

UTIL=$SRC/util
MEM=$SRC/mem
SCHED=$SRC/sched
INT=$SRC/int
DR=$SRC/driver
TME=$SRC/time
CALL=$SRC/syscall
SYNC=$SRC/sync

ARC_SCHED=$ARC/sched
ARC_CALL=$ARC/syscall
ARC_SYNC=$ARC/sync
ARC_UTIL=$ARC/util
ARC_INT=$ARC/int
ARC_MEM=$ARC/mem

C_FILES="$SRC/main.cpp $SRC/init.cpp $SRC/cxx.cpp $SRC/global.cpp $SRC/mb2.cpp $SRC/gdt.cpp $SRC/kdata.cpp $DR/vga_tty.cpp $DR/ps2.cpp $UTIL/io.cpp $UTIL/string.cpp $UTIL/math.cpp $UTIL/misc.cpp $UTIL/linked_list.cpp $TME/pit.cpp $SYNC/spinlock.cpp $MEM/mem.cpp $MEM/pmem.cpp $MEM/vmem.cpp $MEM/kmem.cpp $MEM/slab.cpp $SCHED/process.cpp $SCHED/thread.cpp $INT/pic.cpp $INT/int.cpp $CALL/syscall.cpp $ARC/common.cpp"

ASM_FILES="$SRC/resources.asm $ARC/mb2.asm $ARC/boot.asm $ARC/long_init.asm $ARC/common.asm $ARC/special.asm $ARC_SCHED/thread.asm $ARC_CALL/syscall.asm $ARC_SYNC/spinlock.asm $ARC_UTIL/math.asm $ARC_INT/int.asm $ARC_MEM/vmem.asm"

OUT_FILE=$BUILDDIR/iso/boot/kernel.bin

LD_SCRIPT=$ARC/linker.ld


O="-O2"
EXT=".o"
[[ $1 = debug ]] && G_DEF="-D debug" G=-g && O=-O0 && EXT=".g.o"

COMP_FLAGS="-c -I$INCL/ -ffreestanding -fno-rtti -fno-exceptions -Wall -Wextra -mno-red-zone -mgeneral-regs-only -mcmodel=large -D $ARCH $G_DEF"
ASM_FLAGS="-I$INCL/arch/$ARCH/ -f elf64 -F dwarf"
LINK_FLAGS="-ffreestanding -nostdlib -lgcc -mcmodel=large -n -T $LD_SCRIPT"


for C_FILE in $C_FILES
do
	O_FILES="$O_FILES $BUILDDIR/$C_FILE.o"
	G_O_FILES="$G_O_FILES $BUILDDIR/$C_FILE.g.o"
done

for ASM_FILE in $ASM_FILES
do
	O_FILES="$O_FILES $BUILDDIR/$ASM_FILE.o"
	G_O_FILES="$G_O_FILES $BUILDDIR/$ASM_FILE.g.o"
done

# make directories for gcc, otherwise it complains
mkdir -p $(tree -fid src/ | awk -v builddir="$BUILDDIR/" '{if ($0 == "") exit 0; print builddir $1}')

# src first, then object file, returns 0 if needed to build file
function comp_time {
	TEMP=$BUILDDIR/$1$EXT
	[[ -e $TEMP ]] || return 0
	DEPS="$(awk 'BEGIN {FS="<"; ORS=" "} /^#include/ {gsub (/>/, ""); print "include/" $2}' $1) $1"
	for DEP in $DEPS
	do
		[[ $DEP -nt $TEMP ]] && return 0
	done
}

function c_build {
	if comp_time $1
	then
		echo -e "\nCompiling $1..."
	#probably bad idea to use large code model, but im to lazy to change pagind now
		amd64-elf-g++ $1 -o $BUILDDIR/$1$EXT $G $O $COMP_FLAGS
	else
		return 0
	fi
}

function asm_build {
	if comp_time $1
	then
		echo -e "\nAssembling $1..."
		nasm $1 -o $BUILDDIR/$1$EXT $G $ASM_FLAGS
	else
		return 0
	fi
}

function build {
	for C_FILE in $C_FILES
	do
		c_build $C_FILE || return 1
	done

	for ASM_FILE in $ASM_FILES
	do
		asm_build $ASM_FILE || return 1
	done

	if [[ $EXT = ".g.o" ]]
	then
		amd64-elf-g++ $G_O_FILES -o $OUT_FILE $LINK_FLAGS
	else
		amd64-elf-g++ $O_FILES -o $OUT_FILE $LINK_FLAGS
	fi
}

function build_subdirs {
	for SUBDIR in $SUBDIRS
	do
		$SUBDIR/build.sh $1 || return 1
	done
}

if [[ $1 = clean ]]
then
	build_subdirs clean
	rm -rf $BUILDDIR/$SRC $OUT_FILE $BUILDDIR/kernel.iso
	exit 0
elif build_subdirs $1 && build
then
	grub-mkrescue -d /usr/lib/grub/i386-pc -o $BUILDDIR/kernel.iso $BUILDDIR/iso
	echo kernel built

	[[ $1 = run ]] && qemu-system-x86_64 -m 5120 -cdrom $BUILDDIR/kernel.iso

	if [[ $1 = debug ]]
	then
		#termite qemu-system-x86_64 -m 5120 -debugcon stdio -s -S $BUILDDIR/kernel.iso & $TERM -e "gdb -x debug.gdb"
		qemu-system-x86_64 -m 5120 -debugcon stdio -s -S $BUILDDIR/kernel.iso & $TERM -e "/bin/gdb" "-x" "debug.gdb"
	fi

	if [[ $1 = bochs ]]
	then
		$TERM -e bochs -f bochsrc
	fi
	exit 0
else
	echo kernel build failed
	exit 1
fi
