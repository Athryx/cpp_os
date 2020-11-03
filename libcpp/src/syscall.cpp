#include <syscall.hpp>
#include <types>


#define __SYS_THREAD_NEW 2
#define __SYS_THREAD_BLOCK 3
#define __SYS_EXIT 4
#define __SYS_CREATE_SEMAPHORE 5
#define __SYS_DELETE_SEMAPHORE 6
#define __SYS_SEMAPHORE_LOCK 7
#define __SYS_SEMAPHORE_TRY_LOCK 8
#define __SYS_SEMAPHORE_UNLOCK 9



bool sys::thread_new (thread_func_t func)
{
	return (bool) syscall (__SYS_THREAD_NEW, (usize) func);
}

void sys::thread_block (usize reason, usize nsec)
{
	syscall (__SYS_THREAD_BLOCK, reason, nsec);
}

void sys::exit (usize code)
{
	syscall (__SYS_EXIT, code);
}

usize sys::create_semaphore (usize n)
{
	syscall (__SYS_CREATE_SEMAPHORE, n);
}

bool sys::delete_semaphore (usize id)
{
	syscall (__SYS_DELETE_SEMAPHORE, id);
}

void sys::semaphore_lock (usize id)
{
	syscall (__SYS_SEMAPHORE_LOCK, id);
}

bool sys::semaphore_try_lock (usize id)
{
	return (bool) syscall (__SYS_SEMAPHORE_TRY_LOCK, id);
}

void sys::semaphore_unlock (usize id)
{
	syscall (__SYS_SEMAPHORE_UNLOCK, id);
}
