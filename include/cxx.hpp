#pragma once


#include <types.hpp>


inline void *operator new (usize, void *addr) { return addr; }
inline void *operator new[] (usize, void *addr) { return addr; }
inline void operator delete (void *, void *) {}
inline void operator delete[] (void *, void *) {}
