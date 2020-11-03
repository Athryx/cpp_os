#include <syscall.hpp>
#include <types>


enum sys_num
{
	thread_new = 2,
	thread_block = 3,
	exit = 4,
	create_semaphore = 5,
	delete_semaphore = 6,
	semaphore_lock = 7,
	semaphore_try_lock = 8,
	semaphore_unlock = 9
};
