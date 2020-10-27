#include <sched/process.hpp>
#include <types.hpp>
#include <sched/elf_def.hpp>
#include <util/misc.hpp>
#include <util/string.hpp>
#include <util/io.hpp>
#include <mem/mem.hpp>


sched::process *proc_c = NULL;


sched::process::process ()
{
}

sched::process::~process ()
{
	// once thread_block is added, mark them to be deleted
	for (usize i = 0; i < threads.get_len (); i ++)
	{
	}
}

sched::process *sched::process::load_elf (void *program, usize len)
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

	if (hdr->p_hdr < prgrm || hdr->p_hdr + hdr->p_hdr_n * hdr->p_hdr_entry_size)
	{
		error("Program header list is not in elf memory region")
		return NULL;
	}

	process *out = new process;

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

		memcpy (mem, (void *) (prgrm + p_hdr.p_offset), p_hdr.p_filesz);
		out->addr_space.map_at ((usize) mem, p_hdr.p_vaddr, p_hdr.p_memsz);
	}

	thread *init_thread = new thread (*out, (thread_func_t) hdr->entry);
	if (init_thread == NULL)
	{
		delete out;
		error("Could not create first thread of new process")
		return NULL;
	}

	if (!out->threads.append (init_thread))
	{
		delete init_thread;
		delete out;
		error("Could not append first thread of process to thread list")
		return NULL;
	}

	return out;
}
