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
	sched::thread_c->regs.rsp = data->rsp;
	sched::thread_c->regs.rflags = data->rflags;

	u64 nsec = time_nsec_nolatch ();

	lock_all ();

	for (usize i = 0; i < t_list[T_SLEEP].get_len (); i ++)
	{
		sched::thread *temp = (sched::thread *) t_list[T_SLEEP][i];
		if (temp->get_sleep_time () <= nsec)
		{
			// unblock will set state correctly
			((sched::thread *) t_list[T_SLEEP].remove (i))->unblock ();
			i --;
		}
	}

	unlock_all ();

	if (nsec - nsec_last >= THREAD_TIME_SLICE)
	{
		lock ();
		auto *out = sched::schedule ();
		unlock ();
		return out;
	}

	return NULL;
}

// FIXME: don't let this be called from userspace
static sched::registers *sched_int_handler (int_data *data, error_code_t error_code, sched::registers *regs){
	sched::thread_c->regs = *regs;
	sched::thread_c->regs.rip = data->rip;
	sched::thread_c->regs.rflags = data->rflags;

	lock ();
	auto *out = sched::schedule ();
	unlock ();

	return out;
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
	reg_int_handler (IRQ_TIMER, sched_time_handler, int_handler_type::reg);
	reg_int_handler (INT_SCHED, sched_int_handler, int_handler_type::reg);
}

sched::registers *sched::schedule ()
{
	if (thread_post_c != 0)
	{
		thread_post_flag = true;
		return NULL;
	}

	while (t_list[T_DESTROY].get_len () != 0 && t_list[T_DESTROY][0] != thread_c)
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

		auto &proc_c_save = proc_c ();

		if (thread_c->state == T_RUNNING)
		{
			// appending to this list will remove it from other list
			t_list[T_READY].append (thread_c);
			thread_c->state = T_READY;
		}

		thread_c = (thread *) t_list[T_READY].pop_start ();
		t_list[T_RUNNING].append (thread_c);
		thread_c->state = T_RUNNING;

		if (&proc_c () != &proc_c_save)
			set_cr3 (proc_c ().addr_space.get_cr3 ());

		return &thread_c->regs;
	}

	return NULL;
}


sched::thread::thread (process &proc, thread_func_t func)
: proc (proc),
stack_size (STACK_INITIAL_SIZE),
process_alive (true)
{
	init (func);
}

sched::thread::thread (process &proc, thread_func_t func, usize stack_size)
: proc (proc),
stack_size (stack_size),
process_alive (true)
{
	init (func);
}

sched::thread::~thread ()
{
	if (process_alive)
	{
		proc.addr_space.free ((void *) stack_start);
		proc.add_thread (*this);
	}
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

void sched::thread::init (thread_func_t func)
{
	// if the idle thread has been created
	static bool init_done = false;

	if (proc.get_uid () == 0)
		regs = kernel_regs;
	else if (proc.get_uid () == 1)
		regs = superuser_regs;
	else
		regs = user_regs;

	if (init_done)
	{
		usize u_flag = proc.get_uid () ? V_USER : 0;
		stack_start = (usize) proc.addr_space.alloc (stack_size, V_WRITE | V_XD | u_flag);
		// this should maybe be done better in future
		// FIXME: might break if bad timing, this needs to be locked
		if (&proc_c ().addr_space == &proc.addr_space)
			proc.addr_space.sync_tlb (stack_start);

		regs.rsp = stack_start + stack_size;
		regs.rip = (usize) func;

		t_list[T_READY].append (this);
		state = T_READY;
	}
	else
	{
		stack_start = 0;

		t_list[T_RUNNING].append (this);
		thread_c = this;
		state = T_RUNNING;

		init_done = true;
	}

	if (!proc.add_thread (*this))
	{
		error("Could not add thread to thread list");
		panic ("Recovering from this error is unimplmented");
	}
}
