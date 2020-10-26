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
	};

	class thread : public util::list_node
	{
		public:
			thread (process &proc, thread_func_t func);
			thread (process &proc);

			void switch_to ();

		private:
			process &proc;
			registers regs;

			usize stack_start;
			usize stack_size;

			u64 time;
			u64 sleep_time;
			u8 state;
	};
}
