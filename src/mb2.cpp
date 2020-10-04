#include <mb2.hpp>
#include <types.hpp>
#include <util/misc.hpp>


using namespace mb2;


void *mb2::get (void *mb2_table, u8 type)
{
	// more portable to different endianass with u32
	u32 *mb2 = (u32 *) mb2_table;
	usize i = 2;
	usize b_i = 8;
	for (;;)
	{
		if (mb2[i] == type)
			return mb2 + i;
		if (mb2[i] == 0)
			return NULL;

		b_i += mb2[i + 1];
		b_i = align_up (b_i, 8);
		i = b_i / 4;
	}
}
