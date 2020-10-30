#pragma once


// followed by color byte
#define TTY_SET_COLOR 0x01
// followed by column, row byte
#define TTY_SET_POSITION 0x02

#define ARROW_LEFT 0x11
#define ARROW_RIGHT 0x12
#define ARROW_UP 0x13
#define ARROW_DOWN 0x14
#define HOME 0x15
#define END 0x16
#define PAGE_UP 0x17
#define PAGE_DOWN 0x18
// idk if this is technicly the right code for delete, but i dont really care
#define DELETE 0x7f
