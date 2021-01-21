#include <sched/thread.hpp>
#include <types.hpp>
#include <sched/process.hpp>
#include <sched/sched_def.hpp>
#include <int.hpp>
#include <syscall.hpp>
#include <util/time.hpp>
#include <util/math.hpp>
#include <util/linked_list.hpp>
#include <syscall.hpp>
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

static void thread_cleaner (void);

extern "C" void int_sched (void);


static sched::registers *sched_time_handler (int_data *data, error_code_t error_code, sched::registers *regs)
{
	sched::thread_c->regs = *regs;

	u64 nsec = time_nsec_nolatch ();

	lock_all ();

	for (usize i = 0; i < t_list[T_SLEEP].get_len (); i ++)
	{
		sched::thread *temp = (sched::thread *) t_list[T_SLEEP][i];
		if (temp->get_sleep_time () <= nsec)
		{
			// unblock will set state correctly
			((sched::thread *) t_list[T_SLEEP][i])->unblock ();
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

static sched::registers *sched_int_handler (int_data *data, error_code_t error_code, sched::registers *regs)
{
	// this should be called while locked
	// interrupt was called from userspace
	if (data->cs & 0b11)
		return NULL;

	sched::thread_c->regs = *regs;

	auto *out = sched::schedule ();

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

static void thread_cleaner (void)
{
	for (;;)
	{
		for (;;)
		{
			lock ();
			if (!t_list[T_DESTROY].get_len ())
				break;
			auto *temp = (sched::thread *) t_list[T_DESTROY].pop_start ();
			unlock ();

			if (temp->get_stack_size () == 0)
				panic ("tried to free the idle thread");

			delete temp;
		}
		unlock ();

		// sleep for 0.3 seconds
		sched::thread_c->nsleep (300000000);
	}
}

void sched::init ()
{
	reg_int_handler (IRQ_TIMER, sched_time_handler, int_handler_type::reg);
	reg_int_handler (INT_SCHED, sched_int_handler, int_handler_type::reg);
	if (proc_c ().new_thread (thread_cleaner) == NULL)
		panic ("Could not create cleaner thread");
}

bool sched::sys_thread_new (syscall_vals_t *vals, u32 options, thread_func_t func)
{
	thread *new_thread = proc_c ().new_thread (func);
	if (new_thread == NULL)
		return false;
	else
		return true;
}

void sched::sys_thread_block (syscall_vals_t *vals, u32 options, usize reason, usize nsec)
{
	if (reason > T_REASON_MAX || reason == T_RUNNING)
		return;

	switch (reason)
	{
		case T_SLEEP:
			thread_c->nsleep_until (nsec);
			break;
		default:
			thread_c->block (reason);
	}
}

sched::registers *sched::schedule ()
{
	if (thread_post_c != 0)
	{
		thread_post_flag = true;
		return NULL;
	}

	if (t_list[T_READY].get_len () != 0)
	{
		thread_c->update_time ();

		auto &proc_c_save = proc_c ();

		if (thread_c->state == T_RUNNING)
			thread_c->move_to (T_READY);

		// TODO: add premptive logic here
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
		if (kstack_size != 0)
		{
			proc.addr_space.free ((void *) kstack_start);
		}
		proc.rem_thread (*this);
	}
}

void sched::thread::block (u8 state)
{
	lock ();
	move_to (state);
	if (this == thread_c)
		int_sched ();
	unlock ();
}

void sched::thread::unblock ()
{
	lock ();
	move_to (T_READY);

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

void sched::thread::move_to (usize state_list)
{
	if (state == state_list)
		return;
	if (state != T_SEMAPHORE_WAIT)
		t_list[state].remove_p (this);
	state = state_list;
	if (state_list != T_SEMAPHORE_WAIT)
		t_list[state_list].append (this);
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

		if (proc.get_uid () != 0)
		{
			stack_start = (usize) proc.addr_space.alloc (stack_size, V_XD | V_WRITE | u_flag);

			kstack_size = KSTACK_SIZE;
			// kstack_start = (usize) proc.addr_space.alloc (kstack_size, V_XD | V_WRITE);
			kstack_start = (usize) mem::kalloc (kstack_size);
			// this one should be 16 byte aligned
			regs.kernel_rsp = kstack_start + kstack_size;
		}
		else
		{
			// TODO: make a better fix than this
			stack_start = (usize) mem::kalloc (stack_size);
			kstack_size = 0;
			kstack_start = 0;
		}

		// system v abi says stack needs to be 16 byte aligned before call, so this - 8 is like ret address on stack
		regs.rsp = stack_start + stack_size - 8;
		regs.rip = (usize) func;

		lock ();
		t_list[T_READY].append (this);
		state = T_READY;
		unlock ();
	}
	else
	{
		// no need to lock in init, interrupts are disabled
		// TODO: might need to lock once smp is added
		stack_start = 0;

		t_list[T_RUNNING].append (this);
		thread_c = this;
		state = T_RUNNING;

		init_done = true;
	}

	assert(proc.add_thread (*this));
}


sched::semaphore::semaphore (usize n)
: max_thr_c (n),
thr_c (0)
{
}

sched::semaphore::~semaphore ()
{
}

void sched::semaphore::reset_ready ()
{
	::lock ();
	while (t_waiting.get_len () != 0)
		((sched::thread *) t_waiting.pop_start ())->unblock ();
	::unlock ();
	delete this;
}

void sched::semaphore::reset_destroy ()
{
	::lock ();
	while (t_waiting.get_len () != 0)
		((sched::thread *) t_waiting.pop_start ())->move_to (T_DESTROY);
	::unlock ();
	delete this;
}

void sched::semaphore::lock ()
{
	::lock ();
	if (thr_c < max_thr_c)
	{
		thr_c ++;
		::unlock ();
		return;
	}

	thread_c->move_to (T_SEMAPHORE_WAIT);
	t_waiting.append (thread_c);
	int_sched ();
	::unlock ();
}

bool sched::semaphore::try_lock ()
{
	::lock ();
	if (thr_c < max_thr_c)
	{
		thr_c ++;
		::unlock ();
		return true;
	}
	::unlock ();
	return false;
}

void sched::semaphore::unlock ()
{
	::lock ();
	// if this is true, thr_c already equals max_thr_c
	// this unblock will never cause a thread switch
	if (t_waiting.get_len () != 0)
		((sched::thread *) t_waiting.pop_start ())->unblock ();
	else if (thr_c != 0)
		thr_c --;
	::unlock ();
}
