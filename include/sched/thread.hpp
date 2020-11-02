#pragma once


#include <types.hpp>
#include <util/linked_list.hpp>


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
		// ds is value for all the data segments
		u16 ds;
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
			.ds = 0x1b
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
			.ds = 0x1b
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
			.ds = 0x10
			};

		public:
			// TODO: make stack regrowable
			// TODO: put nonexistant page on end of stack to prevent stack overflows
			thread (process &proc, thread_func_t func);
			thread (process &proc, thread_func_t func, usize stack_size);
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

			void move_to (usize state_list);

			inline usize get_sleep_time () { return sleep_time; }

			inline usize get_stack_size () { return stack_size; }

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
	};

	// TODO: make mutex
	// FIXME: put spinlock in
	class semaphore
	{
		public:
			semaphore (usize n);
			~semaphore ();

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

	bool sys_thread_new (thread_func_t func);
	void sys_thread_block (usize reason, usize nsec);

	registers *schedule ();
}
