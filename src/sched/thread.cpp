#include <sched/thread.hpp>
#include <types.hpp>
#include <sched/process.hpp>
#include <sched/sched_def.hpp>
#include <int.hpp>
#include <util/time.hpp>
#include <util/linked_list.hpp>
#include <arch/x64/common.hpp>


sched::thread *sched::thread_c = NULL;
util::linked_list t_list[16];

u32 thread_post_c = 0;
bool thread_post_flag = false;

// used by update_time and sched_time_handler to calculate how much time has elapsed since last thread switch
static u64 nsec_last = 0;


static sched::registers *sched_time_handler (int_data *data, error_code_t error_code, sched::registers *regs);
static sched::registers *sched_int_handler (int_data *data, error_code_t error_code, sched::registers *regs);

static void lock (void);
static void unlock (void);
static void lock_all (void);
static void unlock_all (void);

extern "C" void int_sched (void);


static sched::registers *sched_time_handler (int_data *data, error_code_t error_code, sched::registers *regs)
{
	sched::thread_c->regs = *regs;
	sched::thread_c->regs.rip = data->rip;
	sched::thread_c->regs.rflags = data->rflags;

	// FIXME: currently this will be innacurate because this interrupt function is called first
	u64 nsec = time_nsec_nolatch ();

	lock_all ();

	for (usize i = 0; i < t_list[T_SLEEP].get_len (); i ++)
	{
		sched::thread *temp = (sched::thread *) t_list[T_SLEEP].get (i);
		if (temp->get_sleep_time () <= nsec)
		{
			// unblock will set state correctly
			((sched::thread *) t_list[T_SLEEP].remove (i))->unblock ();
			i --;
		}
	}

	if (nsec - nsec_last >= THREAD_TIME_SLICE)
	{
		lock ();
		auto *out = sched::schedule ();
		unlock ();
		return out;
	}

	unlock_all ();

	return NULL;
}

static sched::registers *sched_int_handler (int_data *data, error_code_t error_code, sched::registers *regs){
	sched::thread_c->regs = *regs;
	sched::thread_c->regs.rip = data->rip;
	sched::thread_c->regs.rflags = data->rflags;

	return sched::schedule ();
}

// FIXME: make these actually safe
static void lock (void)
{
	cli_safe ();
}

static void unlock (void)
{
	sti_safe ();
}

// FIXME: I don't think lock_all and unlock_all are atomic either
static void lock_all (void)
{
	lock ();
	thread_post_c ++;
}

static void unlock_all (void)
{
	thread_post_c = max(thread_post_c, (u32) 1) - 1;

	if (thread_post_c == 0 && thread_post_flag == true)
	{
		thread_post_flag = false;
		int_sched ();
	}

	unlock ();
}

void sched::init ()
{
	reg_int_handler (IRQ_TIMER, sched_time_handler, true);
	reg_int_handler (INT_SCHED, sched_int_handler, true);
}

sched::registers *sched::schedule ()
{
	if (thread_post_c != 0)
	{
		thread_post_flag = true;
		return NULL;
	}

	while (t_list[T_DESTROY].get_len () != 0 && t_list[T_DESTROY].get (0) != thread_c)
	{
		thread *temp = (thread *) t_list[T_DESTROY].pop_start ();
		// don't free the idle thread
		if (temp->get_stack_size () == 0)
			panic ("tried to free the idle thread");

		delete temp;
	}

	if (t_list[T_READY].get_len () != 0)
	{
		thread_c->update_time ();

		if (thread_c->state == T_RUNNING)
		{
			// appending to this list will remove it from other list
			t_list[T_READY].append (thread_c);
			thread_c->state = T_READY;
		}

		thread_c = (thread *) t_list[T_READY].pop_start ();
		thread_c->state = T_RUNNING;

		return &thread_c->regs;
	}

	return NULL;
}


sched::thread::thread (process &proc, thread_func_t func)
: proc (proc)
{
}

sched::thread::~thread ()
{
}

void sched::thread::block (u8 state)
{
	lock ();
	t_list[state].append (this);
	this->state = state;
	int_sched ();
	unlock ();
}

void sched::thread::unblock ()
{
	lock ();
	t_list[T_READY].append (this);
	state = T_READY;

	// preempt the idle thread, since len 1 means only this thread is running
	if (t_list[T_READY].get_len () == 1)
	{
		int_sched ();
	}

	unlock ();
}

void sched::thread::sleep (u64 time)
{
	nsleep (time * 1000000000);
}

void sched::thread::nsleep (u64 time)
{
	nsleep_until (time_nsec () + time);
}

void sched::thread::nsleep_until (u64 time)
{
	if (time <= time_nsec ())
		return;

	sleep_time = time;
	block (T_SLEEP);
}

u64 sched::thread::update_time ()
{
	u64 nsec_current = time_nsec ();
	thread_c->time += nsec_current - nsec_last;
	nsec_last = nsec_current;
	return thread_c->time;
}

u64 sched::thread::update_time (u64 nsec)
{
	u64 nsec_current = nsec;
	thread_c->time += nsec_current - nsec_last;
	nsec_last = nsec_current;
	return thread_c->time;
}
