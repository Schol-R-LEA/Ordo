#ifndef TERMINAL_H
#define TERMINAL_H
#include <stdint.h>


struct TextCell {
    volatile unsigned char ch;
    volatile uint8_t attribute;
};

extern volatile const struct TextCell* text_buffer;


void advance_cursor();
void gotoxy(uint16_t x, uint16_t y);
void kprintc(char c, uint8_t attrib);
void kprint(const char *string, uint8_t attribs);
void clear_screen();

#endif