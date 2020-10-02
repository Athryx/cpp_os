#include <sync/spinlock.hpp>
#include <types.hpp>


extern "C" void spin_lock (volatile u8 *locked);
extern "C" u8 spin_try_lock (volatile u8 *locked);


spinlock::spinlock ()
{
	locked = 0;
}

void spinlock::lock ()
{
	spin_lock (&locked);
}

bool spinlock::try_lock ()
{
	return (bool) spin_try_lock (&locked);
}

void spinlock::unlock ()
{
	locked = 0;
}
