#ifndef __DRIVER_KBD_H__
#define __DRIVER_KBD_H__


#include <types.h>
#include <def/keys.h>


#define KBD_BUF_SIZE 32


u8 kbd_init (void);

u8 kbd_read (char *buf, u8 n);


#endif
