#pragma once


#include <types.hpp>
#include <sched/thread.hpp>
#include <mem/vmem.hpp>
#include <util/linked_list.hpp>
#include <util/linked_list2.hpp>
#include <util/array_list.hpp>
#include <syscall.hpp>


namespace sched
{
	// TODO: use better data structure for storing semaphores
	class process : public util::list_node
	{
		public:
			process (u8 uid);
			~process ();

			// TODO: add dynamic linking
			// TODO: handle align in future
			// TODO: add thread local storage using fsbase msr
			static process *load_elf (void *program, usize len, u8 uid);

			thread *new_thread (thread_func_t func);
			thread *new_thread (const char *name, thread_func_t func);
			bool add_thread (thread &thr);
			void rem_thread (thread &thr);

			usize create_semaphore (usize n);
			bool delete_semaphore (usize id);
			semaphore *get_semaphore (usize id);

			inline u8 get_uid () { return uid; }

			mem::addr_space addr_space;
		private:
			// TODO: to explicitly delete any members that require deleteing at the end of destructor
			util::linked_list2<thread> threads;
			// next free semaphore id
			usize free_id;
			util::array_list<semaphore> semaphores;

			u8 uid;
	};


	extern util::linked_list *proc_list;


	void proc_init (void);

	void sys_exit (void);

	usize sys_create_semaphore (syscall_vals_t *vals, u32 options, usize n);
	usize sys_delete_semaphore (syscall_vals_t *vals, u32 options, usize id);
	void sys_semaphore_lock (syscall_vals_t *vals, u32 options, usize id);
	bool sys_semaphore_try_lock (syscall_vals_t *vals, u32 options, usize id);
	void sys_semaphore_unlock (syscall_vals_t *vals, u32 options, usize id);

	// only call if thread_c initialized
	inline process &proc_c (void) { return thread_c->proc; }
}

namespace elf
{
	struct header
	{
		u32 magic; // 0x7f than ELF in ascii
		u8 bits; // 1 is 32 bit, 2 is 64 bit
		u8 endian; // 1 is little endian, 2 is big endian
		u8 header_version;
		u8 abi; // 0 for system v
		u64 unused;
		u16 info; // 1 for relocatable, 2 for executable, 3 for shared library, 4 for core
		u16 arch; // 0x3 for x86, 0x3e for x64
		u32 elf_version;
		usize entry;
		usize p_hdr; // program header
		usize s_hdr; // section header
		u32 flags; // x86 doesn't have any flags, can ignore
		u16 hdr_size;
		u16 p_hdr_entry_size;
		u16 p_hdr_n; // number of entries in program header
		u16 s_hdr_entry_size;
		u16 s_hdr_n; // number of entries in section header
		u16 s_hdr_name_i; // index of section header entry that contains the section names
	} __attribute__ ((packed));

	struct p_header
	{
		u32 type; // 0 for null, 1 for load, 2 for dynamic, 3 for interpreter, 4 for notes
		#ifdef x64
		u32 flags; // bitmask, 1 for executable, 2 for writable, 4 for readable
		#endif
		usize p_offset;
		usize p_vaddr;
		usize unused;
		usize p_filesz;
		usize p_memsz;
		usize align; // must be power of 2
	} __attribute__ ((packed));
}
