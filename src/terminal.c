#include <stdint.h>
#include "terminal.h"


volatile const struct TextCell* text_buffer = (struct TextCell *)0xB8000;

const uint16_t MAXH = 80, MAXV = 25;
uint16_t currv = 0, currh = 0;
struct TextCell* text_cursor = (struct TextCell *)0xB8000;


void advance_cursor()
{
    text_cursor++;
    if (currh < MAXH) {
        currh++;
    }
    else {
        currh = 0;
        if (currv < MAXV) {
            currv++;
        }
        else {
            /* handle scrolling */
        }
    }
}

void gotoxy(uint16_t x, uint16_t y)
{
    struct TextCell* new_pos = (struct TextCell *)(0xB8000 + (MAXV * MAXH));
    text_cursor = (struct TextCell *)((unsigned int) text_cursor + (unsigned int) new_pos);
    currh = x;
    currv = y;
}

void kprintc(char c, uint8_t attrib)
{
    text_cursor->ch = c;
    text_cursor->attribute = attrib;
    advance_cursor();
}

void kprint(const char *string, uint8_t attribs)
{
    int i;
    for (i = 0; string[i] != '\0'; i++) {
        kprintc(string[i], attribs);
    }
}

void clear_screen()
{
    for(int i = 0; i < (MAXH * MAXV); i++) {
        kprintc(' ', 0);
    }
}
