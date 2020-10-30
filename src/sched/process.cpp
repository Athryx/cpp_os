#include <sched/process.hpp>
#include <sched/sched_def.hpp>
#include <types.hpp>
#include <sched/elf_def.hpp>
#include <util/misc.hpp>
#include <util/string.hpp>
#include <util/io.hpp>
#include <mem/mem.hpp>
#include <arch/x64/common.hpp>
#include <arch/x64/special.hpp>


util::linked_list *sched::proc_list;


sched::process::process (u8 uid)
: uid (uid)
{
	proc_list->append (this);
}

sched::process::~process ()
{
	for (usize i = 0; i < threads.get_len (); i ++)
	{
		threads[i]->set_process_alive (false);
		threads[i]->block (T_DESTROY);
	}

	proc_list->remove_p (this);
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

	set_cr3 (proc->addr_space.get_cr3 ());
}
