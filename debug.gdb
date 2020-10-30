set disassembly-flavor intel
add-symbol-file abuild/iso/boot/kernel.bin
add-symbol-file initfs/initfs.bin
break _start
target remote localhost:1234
layout asm
layout next
