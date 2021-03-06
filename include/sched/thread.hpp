#pragma once


#include <types.hpp>
#include <util/linked_list.hpp>
#include <syscall.hpp>
#include <sync/spinlock.hpp>


namespace sched
{
	typedef void (*thread_func_t) ();

	class process;

	struct registers
	{
		usize rax;
		usize rbx;
		usize rcx;
		usize rdx;
		usize rbp;
		usize rsp;
		usize kernel_rsp;
		usize call_save_rsp;
		usize rdi;
		usize rsi;
		usize r8;
		usize r9;
		usize r10;
		usize r11;
		usize r12;
		usize r13;
		usize r14;
		usize r15;
		usize rflags;
		usize rip;
		u16 cs;
		u16 ss;
		void print ();
	} __attribute__ ((packed));

	class thread : public util::list_node
	{
		private:
			static constexpr registers user_regs = {
			.rax = 0,
			.rbx = 0,
			.rcx = 0,
			.rdx = 0,
			.rbp = 0,
			.rsp = 0,
			.kernel_rsp = 0,
			.call_save_rsp = 0,
			.rdi = 0,
			.rsi = 0,
			.r8 = 0,
			.r9 = 0,
			.r10 = 0,
			.r11 = 0,
			.r12 = 0,
			.r13 = 0,
			.r14 = 0,
			.r15 = 0,
			.rflags = 0x3202,
			.rip = 0,
			.cs = 0x23,
			.ss = 0x1b
			};

			static constexpr registers superuser_regs = {
			.rax = 0,
			.rbx = 0,
			.rcx = 0,
			.rdx = 0,
			.rbp = 0,
			.rsp = 0,
			.kernel_rsp = 0,
			.call_save_rsp = 0,
			.rdi = 0,
			.rsi = 0,
			.r8 = 0,
			.r9 = 0,
			.r10 = 0,
			.r11 = 0,
			.r12 = 0,
			.r13 = 0,
			.r14 = 0,
			.r15 = 0,
			.rflags = 0x202,
			.rip = 0,
			.cs = 0x23,
			.ss = 0x1b
			};

			static constexpr registers kernel_regs = {
			.rax = 0,
			.rbx = 0,
			.rcx = 0,
			.rdx = 0,
			.rbp = 0,
			.rsp = 0,
			.kernel_rsp = 0,
			.call_save_rsp = 0,
			.rdi = 0,
			.rsi = 0,
			.r8 = 0,
			.r9 = 0,
			.r10 = 0,
			.r11 = 0,
			.r12 = 0,
			.r13 = 0,
			.r14 = 0,
			.r15 = 0,
			.rflags = 0x202,
			.rip = 0,
			.cs = 0x08,
			.ss = 0x10
			};

		public:
			// TODO: make stack regrowable
			// TODO: put nonexistant page on end of stack to prevent stack overflows
			// NOTE: thread name is only for debugging purposes, it isn't necassary
			thread (process &proc, thread_func_t func);
			thread (process &proc, thread_func_t func, usize stack_size);
			thread (const char *name, process &proc, thread_func_t func);
			thread (const char *name, process &proc, thread_func_t func, usize stack_size);
			~thread ();

			void block (u8 state);
			void unblock ();

			void sleep (u64 time);
			void nsleep (u64 time);
			void nsleep_until (u64 time);

			// schedular should be locked
			u64 update_time ();
			// this version has time passed into it to avoid repeated calls to time_nsec
			u64 update_time (u64 nsec);

			// doesn't do anything if state equals state_list
			// should be called while scheduler is locked
			void move_to (usize state_list);

			void print ();

			inline usize get_sleep_time () { return sleep_time; }

			inline usize get_stack_size () { return stack_size; }

			inline const char *get_name () { return name ? name : "<anonymous_thread>"; }

			inline void set_process_alive (bool in) { process_alive = in; }


			registers regs;

			u64 time;

			u8 state;

			process &proc;
		private:
			void init (thread_func_t);

			usize stack_start;
			usize stack_size;
			usize kstack_start;
			usize kstack_size;

			u64 sleep_time;

			bool process_alive;

			const char *name;
	};

	// TODO: make mutex
	// FIXME: put spinlock in
	class semaphore
	{
		public:
			semaphore (usize n);
			// frees this semaphore and does not move any threads anywhere
			~semaphore ();
			// don't call reset_ready or reset_destroy from thread in list, this shouldn't be possible anyways
			// moves all waiting threads to T_READY state and frees this semaphore
			void reset_ready ();
			// moves all waiting threads to T_DESTROY state and frees this semaphore
			void reset_destroy ();

			void lock ();
			bool try_lock ();
			void unlock ();

		private:
			const usize max_thr_c;
			usize thr_c;
			util::linked_list t_waiting;
	};

	extern sched::thread *thread_c;

	void init ();

	bool sys_thread_new (syscall_vals_t *vals, u32 options, thread_func_t func);
	void sys_thread_block (syscall_vals_t *vals, u32 options, usize reason, usize nsec);

	registers *schedule ();
}
