#!/bin/bash

ARCH=x64

PROJECT=$(dirname $0)
cd $PROJECT

BUILDDIR=abuild

# subdirs are built in order
SUBDIRS=""

SUB_INCL=""
INCL=include

SRC=src
ARC=$SRC/arch/$ARCH

C_FILES="$SRC/syscall.cpp"

ASM_FILES="$ARC/syscall.asm"

OUT_FILE=libcpp.a

LD_SCRIPT=""

O=-O2
EXT=".o"
[[ $1 = debug ]] && G_DEF="-D debug" && G=-g && O=-O0 && EXT=".g.o"

COMP_FLAGS="-I$INCL/ -c -ffreestanding -fno-rtti -fno-exceptions -Wall -Wextra -mno-red-zone -mgeneral-regs-only -D $ARCH $G_DEF"
ASM_FLAGS="-I$INCL/arch/$ARCH/ -f elf64 -F dwarf"
# append $LIB_PATHS to LINK_FLAGS variable definition if you want to use a library
LINK_FLAGS=""

CC=amd64-elf-g++
ASM=nasm
LD="ar rvs"




export LIB_PATHS="$LIB_PATHS -L$(realpath $PROJECT)"

COMP_FLAGS="-I$INCL/ $COMP_FLAGS"
for INCL_FILE in $SUB_INCL_OUT
do
	COMP_FLAGS="-I$INCL_FILE $COMP_FLAGS"
done
export SUB_INCL_OUT="$(realpath $SUB_INCL) $SUB_INCL_OUT"

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
		$CC $1 -o $BUILDDIR/$1$EXT $G $O $COMP_FLAGS
	else
		return 0
	fi
}

function asm_build {
	if comp_time $1
	then
		echo -e "\nAssembling $1..."
		$ASM $1 -o $BUILDDIR/$1$EXT $G $ASM_FLAGS
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
		$LD $OUT_FILE $G_O_FILES
	else
		$LD $OUT_FILE $O_FILES
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
	rm -rf $BUILDDIR/$SRC $OUT_FILE
	build_subdirs clean
	exit 0
elif build && build_subdirs $1
then
	echo $OUT_FILE built
	[[ -e "post-build.sh" ]] && ./post-build.sh $1
	exit 0
else
	echo $OUT_FILE build failed
	exit 1
fi
