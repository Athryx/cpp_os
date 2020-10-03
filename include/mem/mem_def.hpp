#pragma once


#include <types.hpp>


#ifdef x64

// 256 gibibytes
#define MAX_SUPPERTED_MEM (1 << 38)
#define MAX_SUPPERTED_MEM_BITS 39
// 4 KiB
#define PAGE_SIZE ((usize) 4096)
#define PAGE_BITS 12

// 15 bits of address used in first order size, 32 KiB
// this is default setting
#define FIRST_ORDER_SIZE (PAGE_SIZE * 8)

#define HEAP_SIZE (FIRST_ORDER_SIZE * 64)

extern usize TEXT_START;
extern usize TEXT_END;
extern usize RODATA_START;
extern usize RODATA_END;
extern usize DATA_START;
extern usize DATA_END;
extern usize BSS_START;
extern usize BSS_END;
extern usize KERNEL_END;
extern usize KERNEL_VMA;
extern usize KERNEL_LMA;

// to make things prettier
extern usize text_start;
extern usize text_end;
extern usize rodata_start;
extern usize rodata_end;
extern usize data_start;
extern usize data_end;
extern usize bss_start;
extern usize bss_end;
extern usize kernel_end;
extern usize kernel_vma;
extern usize kernel_lma;

#endif
