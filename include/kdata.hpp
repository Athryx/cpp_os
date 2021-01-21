#pragma once 
// TODO: update kdata for smp when that is a thing


#include <types.hpp>


namespace kdata
{
	struct __attribute__ ((packed)) gs_data
	{
		usize call_rsp;
		usize call_save_rsp;
	};


	extern gs_data data;


	void init (void);
}
