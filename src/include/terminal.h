#ifndef TERMINAL_H
#define TERMINAL_H
#include <stdint.h>


enum Color: uint8_t
{
    BLACK,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    GRAY,

    DARK_GRAY,
    LT_BLUE,
    LT_GREEN,
    LT_CYAN,
    LT_RED,
    LT_MAGENTA,
    YELLOW,
    WHITE
};


struct TextCell {
    volatile unsigned char ch;
    volatile struct
    {
        enum Color fg_color:4;
        enum Color bg_color:4;
    } attribute;
};

extern volatile const struct TextCell* text_buffer;


void advance_cursor();
void gotoxy(uint16_t x, uint16_t y);
uint16_t get_x();
uint16_t get_y();
void set_fg(enum Color new_fg);
void set_bg(enum Color new_bg);
void kprintc(char c, enum Color fg_color, enum Color bg_color);
void kprints(const char *string, enum Color fg_color, enum Color bg_color);
uint8_t kprintu(const uint32_t i, uint8_t base, enum Color fg_color, enum Color bg_color);
uint8_t kprinti(const int32_t i, uint8_t base, enum Color fg_color, enum Color bg_color);
void kprintlx(const uint64_t i, enum Color fg_color, enum Color bg_color);
void kprintf(const char* format, ...);
void clear_screen();

#endif