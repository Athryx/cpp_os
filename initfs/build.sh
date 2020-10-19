#!/bin/bash

ARCH=x64

PROJECT=$(dirname $0)
cd $PROJECT

BUILDDIR=abuild

SUBDIRS=""

SRC=src
INCL=include
ARC=$SRC/arch/$ARCH

C_FILES="$SRC/main.c"

ASM_FILES=""

OUT_FILE=initfs.bin

LD_SCRIPT=$ARC/linker.ld

O=-O2
EXT=".o"
[[ $1 = debug ]] && G_DEF="-D debug" && G=-g && O=-O0 && EXT=".g.o"

COMP_FLAGS="-c -I$INCL/ -ffreestanding -fno-rtti -fno-exceptions -Wall -Wextra -mno-red-zone -mgeneral-regs-only -D $ARCH $G_DEF"
ASM_FLAGS="-f elf64 -F dwarf"
LINK_FLAGS="-ffreestanding -nostdlib -lgcc -n"

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
	rm -rf $BUILDDIR/$SRC $OUT_FILE
	exit 0
elif build_subdirs $1 && build
then
	echo initfs built
	exit 0
else
	echo initfs build failed
	exit 1
fi
