#include <sched/process.hpp>
#include <sched/thread.hpp>
#include <sched/sched_def.hpp>
#include <types.hpp>
#include <sched/elf_def.hpp>
#include <util/misc.hpp>
#include <util/string.hpp>
#include <util/array_list.hpp>
#include <util/io.hpp>
#include <mem/mem.hpp>
#include <arch/x64/common.hpp>
#include <arch/x64/special.hpp>
#include <syscall.hpp>


util::linked_list *sched::proc_list;


sched::process::process (u8 uid)
: free_id (0),
semaphores (false),
uid (uid)
{
	proc_list->append (this);
}

sched::process::~process ()
{
	proc_list->remove_p (this);

	for (usize i = 0; i < semaphores.get_len (); i ++)
		semaphores[i]->reset_destroy ();

	for (usize i = 0; i < threads.get_len (); i ++)
	{
		threads[i]->set_process_alive (false);
		if (threads[i] != thread_c)
			threads[i]->block (T_DESTROY);
	}

	if (this == &proc_c ())
	{
		// super jank, but should work
		delete &threads;
		delete &semaphores;
		delete &addr_space;
		mem::kfree (this, sizeof (process));
		thread_c->block (T_DESTROY);
	}
}

sched::process *sched::process::load_elf (void *program, usize len, u8 uid)
{
	elf::header *hdr = (elf::header *) program;
	usize prgrm = (usize) program;

	// check if all elf header fields match
	#ifdef x64
	if (hdr->magic != ELF_MAGIC || hdr->bits != ELF_64_BIT || hdr->endian != ELF_LITTLE_ENDIAN || hdr->abi != ELF_SYSTEM_V || hdr->arch != ELF_x64 || hdr->p_hdr_entry_size != sizeof (elf::p_header))
	{
		error("Invalid elf header for x64 architecture")
		return NULL;
	}
	#endif
	// in future check the info data field

	if (hdr->p_hdr + hdr->p_hdr_n * hdr->p_hdr_entry_size > prgrm + len)
	{
		error("Program header list is not in elf memory region")
		return NULL;
	}

	process *out = new process (uid);
	if (out == NULL)
	{
		error("Could not create process object")
		return NULL;
	}

	elf::p_header *p_hdrs = (elf::p_header *) (prgrm + hdr->p_hdr);

	for (usize i = 0; i < hdr->p_hdr_n; i ++)
	{
		// might need to handler alignment, but I would think v_addr is already aligned to desired position
		elf::p_header p_hdr = p_hdrs[i];
		if (p_hdr.type != ELF_P_LOAD)
			continue;

		if (p_hdr.p_offset + p_hdr.p_filesz > len)
		{
			delete out;
			error("Invalid program header in elf program")
			return NULL;
		}

		void *mem = mem::allocz (p_hdr.p_memsz);
		if (mem == NULL)
		{
			delete out;
			error("Not enough memory to load elf program")
			return NULL;
		}

		memcpy ((void *) (((usize) mem) + p_hdr.p_vaddr % PAGE_SIZE), (void *) (prgrm + p_hdr.p_offset), p_hdr.p_filesz);

		usize flags = 0;
		flags |= (p_hdr.flags & ELF_EXEC ? 0 : V_XD);
		flags |= (p_hdr.flags & ELF_WRITE ? V_WRITE : 0);
		flags |= uid == 0 ? 0 : V_USER;

		if (!out->addr_space.map_at ((usize) mem, p_hdr.p_vaddr, p_hdr.p_memsz, flags))
		{
			delete out;
			error("Could not map program region in virtual address space")
			return NULL;
		}
	}

	if (out->new_thread ((thread_func_t) hdr->entry) == NULL)
	{
		delete out;
		error("Could not create first thread of new process")
		return NULL;
	}

	return out;
}

sched::thread *sched::process::new_thread (thread_func_t func)
{
	thread *out = new thread (*this, func);
	if (out == NULL)
		return NULL;

	if (!add_thread (*out))
	{
		delete out;
		return NULL;
	}

	return out;
}

bool sched::process::add_thread (thread &thr)
{
	return threads.append (&thr);
}

void sched::process::rem_thread (thread &thr)
{
	for (usize i = 0; i < threads.get_len (); i ++)
	{
		if (&thr == threads[i])
		{
			threads.remove (i);
			break;
		}
	}

	if (threads.get_len () == 0)
		delete this;
}

usize sched::process::create_semaphore (usize n)
{
	// might be wrong
	auto *sem  = new semaphore (n);
	if (sem == NULL)
		return -1;

	usize i = free_id;
	if (free_id == semaphores.get_len ())
	{
		free_id ++;
	}
	else
	{
		usize j = free_id + 1;
		for (; j < semaphores.get_len (); j ++)
		{
			if (semaphores[j] == NULL)
			{
				free_id = j;
				break;
			}
		}
		if (j == semaphores.get_len ())
			free_id = j;
	}

	if (!semaphores.set (i, sem))
		return -1;

	return i;
}

bool sched::process::delete_semaphore (usize id)
{
	if (semaphores[id] == NULL)
		return false;

	if (id < free_id)
		free_id = id;

	semaphores[id]->reset_ready ();

	semaphores.set (id, NULL);
	return true;
}

sched::semaphore *sched::process::get_semaphore (usize id)
{
	if (semaphores[id] == NULL)
		return NULL;

	return semaphores[id];
}


void sched::proc_init (void)
{
	// allow use of execute diable in pages
	u64 efer_msr = rdmsr (EFER_MSR);
	wrmsr (EFER_MSR, efer_msr | EFER_NXE);

	proc_list = new util::linked_list;
	if (!proc_list)
		panic ("Couldn't make process list");

	process *proc = new process (KUID);
	if (!proc)
		panic ("Couldn't make kernel process data");

	thread *idle_thread = new thread (*proc, NULL, 0);
	if (!idle_thread)
		panic ("Couldn't make idle thread data");

	proc_c ().add_thread (*idle_thread);

	set_cr3 (proc->addr_space.get_cr3 ());
}

void sched::sys_exit (void)
{
	delete &proc_c ();
}

usize sched::sys_create_semaphore (syscall_vals_t *vals, u32 options, usize n)
{
	return proc_c ().create_semaphore (n);
}

usize sched::sys_delete_semaphore (syscall_vals_t *vals, u32 options, usize id)
{
	return proc_c ().delete_semaphore (id);
}

void sched::sys_semaphore_lock (syscall_vals_t *vals, u32 options, usize id)
{
	proc_c ().get_semaphore (id)->lock ();
}

bool sched::sys_semaphore_try_lock (syscall_vals_t *vals, u32 options, usize id)
{
	return proc_c ().get_semaphore (id)->try_lock ();
}

void sched::sys_semaphore_unlock (syscall_vals_t *vals, u32 options, usize id)
{
	proc_c ().get_semaphore (id)->unlock ();
}
