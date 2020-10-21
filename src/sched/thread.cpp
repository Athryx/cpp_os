#include <sched/thread.hpp>
#include <types.hpp>
#include <sched/process.hpp>


sched::thread thread_c (sched::proc_c);


sched::thread::thread (process &proc)
: proc (proc)
{
}

void sched::thread::switch_to ()
{
}
