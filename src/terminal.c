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
    if (c == '\n')
    {
        gotoxy(0, currv + 1);
    }
    else if (c == '\t')
    {
        gotoxy(currh + 4, currv);
    }
    else
    {
        text_cursor->ch = c;
        text_cursor->attribute.fg_color  = fg_color;
        text_cursor->attribute.bg_color  = bg_color;
        text_cursor->attribute.ch_attrib = ch_attrib;
        advance_cursor();
    }
}

void kprints(const char *string, uint8_t fg_color, uint8_t bg_color, uint8_t ch_attrib)
{
    for (char* i = (char *) string; *i != '\0'; i++)
    {
        kprintc(*i, fg_color, bg_color, ch_attrib);
    }
}


uint8_t kprintu(const uint32_t i, uint8_t base, uint8_t fg_color, uint8_t bg_color, uint8_t ch_attrib)
{
    uint32_t dividend = i / base, rem = i % base;
    char value;
    uint8_t count = 1;

    if (dividend != 0)
    {
        count += kprintu(dividend, base, fg_color, bg_color, ch_attrib);
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
    return count;
}


uint8_t kprinti(const int32_t i, uint8_t base, uint8_t fg_color, uint8_t bg_color, uint8_t ch_attrib)
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
    return kprintu(value, base, fg_color, bg_color, ch_attrib) + 1;
}


void kprintlx(const uint64_t i, uint8_t fg_color, uint8_t bg_color, uint8_t ch_attrib)
{
    uint16_t save_h = currh, save_v = currv;
    union
    {
        uint64_t total;
        struct {
        uint32_t lo, hi;
        } sections;
    } value;
    uint8_t count;

    value.total = i;
    count = kprintu(value.sections.hi, 16, fg_color, bg_color, ch_attrib);
    gotoxy(save_h, save_v);
    for (int c = 8 - count; c > 0; c--)
    {
        kprintc('0', fg_color, bg_color, ch_attrib);
    }
    kprintu(value.sections.hi, 16, fg_color, bg_color, ch_attrib);
    save_h = currh;

    count = kprintu(value.sections.lo, 16, fg_color, bg_color, ch_attrib);
    gotoxy(save_h, save_v);
    for (int c = 8 - count; c > 0; c--)
    {
        kprintc('0', fg_color, bg_color, ch_attrib);
    }
    kprintu(value.sections.lo, 16, fg_color, bg_color, ch_attrib);
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
