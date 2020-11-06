#pragma once


// 256 KiB
#define STACK_INITIAL_SIZE (1 << 18)
// 16 MiB
#define STACK_RESRVE_SIZE (1 << 24)

#define KSTACK_SIZE (1 << 18)

// 5 ms for max time slice
#define THREAD_TIME_SLICE 5000000

#define T_RUNNING 0
#define T_READY 1
#define T_DESTROY 2
#define T_SLEEP 3
#define T_REASON_MAX 3

#define T_SEMAPHORE_WAIT 128

// kernel uid
#define KUID 0
// superuser uid
#define SUID 1

#define ELF_EXEC 1
#define ELF_WRITE 2
#define ELF_READ 4
