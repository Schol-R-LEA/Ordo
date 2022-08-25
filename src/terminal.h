#ifndef TERMINAL_H
#define TERMINAL_H
#include <stdint.h>



enum Color
{
    BLACK,
    BLUE,
    GREEN,
    CYAN,
    RED,
    PURPLE,
    BROWN,
    GRAY,
    DARK_GRAY,
    LT_BLUE,
    LT_GREEN,
    LT_CYAN,
    LT_RED,
    LT_PURPLE,
    YELLOW,
    WHITE
};


struct TextCell {
    volatile unsigned char ch;
    volatile struct
    {
        uint8_t fg_color:4,
                bg_color:3,
                ch_attrib:1;

    } attribute;
};

extern volatile const struct TextCell* text_buffer;


void advance_cursor();
void gotoxy(uint16_t x, uint16_t y);
void kprintc(char c, uint8_t fg_color, uint8_t bg_color, uint8_t ch_attrib);
void kprints(const char *string, uint8_t fg_color, uint8_t bg_color, uint8_t ch_attrib);
void kprintu(const uint32_t i, uint8_t base, uint8_t fg_color, uint8_t bg_color, uint8_t ch_attrib);
void kprinti(const int32_t i, uint8_t base, uint8_t fg_color, uint8_t bg_color, uint8_t ch_attrib);
void clear_screen();

#endif