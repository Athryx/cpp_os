#pragma once


#include <types.hpp>


#ifdef x64

#define MAX_MEM_NODES 4

// 256 gibibytes
#define MAX_SUPPERTED_MEM (1 << 38)
#define MAX_SUPPERTED_MEM_BITS 39
// 4 KiB
#define PAGE_SIZE ((usize) 4096)
#define PAGE_BITS 12

// 2 MiB
#define BIG_PAGE_SIZE (PAGE_SIZE << 9)
#define BIG_PAGE_BITS 21

// 1 GiB
#define HUGE_PAGE_SIZE (BIG_PAGE_SIZE << 9)
#define HUGE_PAGE_BITS 30

// 15 bits of address used in first order size, 32 KiB
// this is default setting
#define FIRST_ORDER_SIZE (PAGE_SIZE * 8)

#define BUDDY_ALLOC_MIN_SIZE (FIRST_ORDER_SIZE * 2)

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
