#include <kdata.hpp>
#include <types.hpp>
#include <arch/x64/special.hpp>


kdata::gs_data kdata::data;


void kdata::init (void)
{
	// TODO: use swapgs insted of just keeping it in gsbase register
	wrmsr (GSBASE_MSR, (u64) &data);
}
