#include <cxx.hpp>
#include <mem/kmem.hpp>
#include <util/io.hpp>


extern "C" void __cxa_pure_virtual ()
{
	panic ("Dispach to pure virtual function");
}

void *operator new (usize n)
{
	return mem::kalloc (n);
}

void *operator new[] (usize n)
{
	return mem::kalloc (n);
}
