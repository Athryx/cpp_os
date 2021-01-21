#include <syscall.hpp>
extern "C" void sys_hi ();

void thread_2 (void)
{
	sys_hi ();
	for (;;);
}

int main (void)
{
	//sys::thread_new (thread_2);
	sys_hi ();
	for (;;);
}
