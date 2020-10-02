#pragma once


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

#endif
