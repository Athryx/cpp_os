#include <sched/thread.hpp>
#include <types.hpp>
#include <sched/process.hpp>


// find out how to fix linker errors
//sched::thread thread_c (sched::proc_c);


sched::thread::thread (process &proc, thread_func_t func)
: proc (proc)
{
}

sched::thread::thread (process &proc)
: proc (proc)
{
}

void sched::thread::switch_to ()
{
}
