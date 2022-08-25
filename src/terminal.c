#include <stdint.h>
#include "terminal.h"


#define BUFFER 0xB8000

volatile const struct TextCell* text_buffer = (struct TextCell *)BUFFER;

const uint16_t MAXH = 80, MAXV = 25, MAX_TEXT = MAXH * MAXV;
uint16_t currv = 0, currh = 0;
struct TextCell* text_cursor = (struct TextCell *)BUFFER;


void advance_cursor()
{
    text_cursor++;
    if (currh < MAXH)
    {
        currh++;
    }
    else
    {
        currh = 0;
        if (currv < MAXV)
        {
            currv++;
        }
        else
        {
            currv = MAXV - 1;
            /* handle scrolling */
        }
    }
}

void gotoxy(uint16_t x, uint16_t y)
{
    text_cursor = (struct TextCell *)((BUFFER + (2 * (x + (MAXH * y)))));
    currh = x;
    currv = y;
}

void kprintc(char c, uint8_t fg_color, uint8_t bg_color, uint8_t ch_attrib)
{
    text_cursor->ch = c;
    text_cursor->attribute.fg_color  = fg_color;
    text_cursor->attribute.bg_color  = bg_color;
    text_cursor->attribute.ch_attrib = ch_attrib;
    advance_cursor();
}

void kprints(const char *string, uint8_t fg_color, uint8_t bg_color, uint8_t ch_attrib)
{
    for (char* i = (char *) string; *i != '\0'; i++)
    {
        kprintc(*i, fg_color, bg_color, ch_attrib);
    }
}


void kprintu(const uint32_t i, uint8_t base, uint8_t fg_color, uint8_t bg_color, uint8_t ch_attrib)
{
    uint32_t dividend = i / base, rem = i % base;
    char value;

    if (dividend != 0)
    {
        kprintu(dividend, base, fg_color, bg_color, ch_attrib);
    }

    if (rem > 9)
    {
        value = (char)(rem - 10 + (uint32_t)'A');
    }
    else
    {
        value = (char)(rem + (uint32_t)'0');
    }
    kprintc(value, fg_color, bg_color, ch_attrib);
}


void kprinti(const int32_t i, uint8_t base, uint8_t fg_color, uint8_t bg_color, uint8_t ch_attrib)
{
    uint32_t value;
    if (i < 0)
    {
        kprintc('-', fg_color, bg_color, ch_attrib);
        value = (uint32_t) (~i + 1);
    }
    else
    {
        value = (uint32_t) i;
    }
    kprintu(value, base, fg_color, bg_color, ch_attrib);
}


void clear_screen()
{
    struct TextCell *temp = (struct TextCell *)0xb8000;
    for(int i = 0; i < MAX_TEXT; i++)
    {
        temp->ch = ' ';
        text_cursor->attribute.fg_color  = GRAY;
        text_cursor->attribute.bg_color  = BLACK;
        text_cursor->attribute.ch_attrib = 0;
        temp++;
    }
    text_cursor = (struct TextCell *)0xb8000;
    currh = 0;
    currv = 0;
}
