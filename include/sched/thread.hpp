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

	// FIXME: fix make sure to switch virt addr
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
			.rflags = 0,
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
			.rflags = 0,
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
			.rflags = 0,
			.rip = 0,
			.cs = 0x08,
			.ds = 0x10
			};
		public:
			// TODO: make stack regrowable
			// TODO: put nonexistant page on end of stack to prevent stack overflows
			thread (process &proc, thread_func_t func);
			// FIXME: implement destructor
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

			inline usize get_sleep_time () { return sleep_time; }

			inline usize get_stack_size () { return stack_size; }


			registers regs;

			u64 time;

			u8 state;
		private:
			process &proc;

			usize stack_start;
			usize stack_size;

			u64 sleep_time;
	};

	extern sched::thread *thread_c;

	void init ();

	registers *schedule ();
}
