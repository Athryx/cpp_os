#pragma once


#include <types.hpp>


// c++ wrapper class around assemly spinlock
class spinlock
{
	public:
		spinlock ();
		void lock ();
		// doesn't spin if it is already locked
		bool try_lock ();
		void unlock ();
	private:
		u8 locked;
};
