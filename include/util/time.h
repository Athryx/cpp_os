#ifndef __UTIL_TIME_H__
#define __UTIL_TIME_H__


#include <types.h>

void time_init (void);
u64 time_nsec (void);
// returns time_nanosec roudned to nearest second
u64 time_sec (void);

u64 time_nsec_nolatch (void);
u64 time_sec_nolatch (void);


#endif
