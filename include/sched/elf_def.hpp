#pragma once


#define ELF_MAGIC 0x7f454c46

#define ELF_32_BIT 1
#define ELF_64_BIT 2

#define ELF_LITTLE_ENDIAN 1
#define ELF_BIG_ENDIAN 2

#define ELF_SYSTEM_V 0

#define ELF_RELOCATABLE 1
#define ELF_EXECUTABLE 2
#define ELF_SHARED 3
#define ELF_CORE 4

#define ELF_x86 3
#define ELF_x64 0x3e

#define ELF_P_NULL 0
#define ELF_P_LOAD 1
#define ELF_P_DYNAMIC 2
#define ELF_P_INTERP 3
#define ELF_P_NOTES 4

#define ELF_P_EXECUTE 1
#define ELF_P_WRITE 2
#define ELF_P_READ 4
