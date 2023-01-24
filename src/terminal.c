#include <stdint.h>
#include <stdarg.h>
#include "terminal.h"
#include "kernel.h"
#include "mem.h"

#define BUFFER 0xB8000

volatile const struct TextCell* text_buffer = (struct TextCell *)BUFFER;

const uint16_t MAXH = 80, MAXV = 25, MAX_TEXT = MAXH * MAXV, BYTES_PER_LINE = MAXH * 2;
uint16_t currv = 0, currh = 0;
struct TextCell* text_cursor = (struct TextCell *)BUFFER;

enum Color current_default_foreground = GRAY;
enum Color current_default_background = BLACK;

void scroll()
{
    for (uint32_t i = 0; i < MAX_TEXT; i += MAXH)
    {
        memcpy((void *) &text_buffer[i], (void *) &text_buffer[i+MAXH], BYTES_PER_LINE);
    }

    // reset the cursor to the start of the last line
    gotoxy(0, MAXV - 1);
    memset((void *) &text_buffer[currv], 0, BYTES_PER_LINE);
}


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
        currv++;
        if (currv >= MAXV - 1)
        {
            scroll();
        }
    }
}


void gotoxy(uint16_t x, uint16_t y)
{
    x = (x < MAXH) ? x : MAXH - 1;
    y = (y < MAXV) ? y : MAXV - 1;
    text_cursor = (struct TextCell *)((BUFFER + (2 * (x + (MAXH * y)))));
    currh = x;
    currv = y;
}


void set_fg(enum Color new_fg)
{
    current_default_foreground = new_fg;
}


void set_bg(enum Color new_bg)
{
    current_default_background = new_bg;
}


void kprintc(char c, enum Color fg_color, enum Color bg_color)
{
    if (c == '\n')
    {
        if (currv >= MAXV - 1)
        {
            scroll();
        }
        else
        {
            gotoxy(0, currv + 1);
        }
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
        advance_cursor();
    }
}

void kprints(const char *string, enum Color fg_color, enum Color bg_color)
{
    for (char* i = (char *) string; *i != '\0'; i++)
    {
        kprintc(*i, fg_color, bg_color);
    }
}


uint8_t kprintu(const uint32_t i, uint8_t base, enum Color fg_color, enum Color bg_color)
{
    uint32_t dividend = i / base, rem = i % base;
    char value;
    uint8_t count = 1;

    if (dividend != 0)
    {
        count += kprintu(dividend, base, fg_color, bg_color);
    }

    if (rem > 9)
    {
        value = (char)(rem - 10 + (uint32_t)'A');
    }
    else
    {
        value = (char)(rem + (uint32_t)'0');
    }
    kprintc(value, fg_color, bg_color);
    return count;
}


uint8_t kprinti(const int32_t i, uint8_t base, enum Color fg_color, enum Color bg_color)
{
    uint32_t value;
    if (i < 0)
    {
        kprintc('-', fg_color, bg_color);
        value = (uint32_t) (~i + 1);
    }
    else
    {
        value = (uint32_t) i;
    }
    return kprintu(value, base, fg_color, bg_color) + 1;
}


void kprintlx(const uint64_t i, enum Color fg_color, enum Color bg_color)
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
    count = kprintu(value.sections.hi, 16, fg_color, bg_color);
    gotoxy(save_h, save_v);
    for (int c = 8 - count; c > 0; c--)
    {
        kprintc('0', fg_color, bg_color);
    }
    kprintu(value.sections.hi, 16, fg_color, bg_color);
    save_h = currh;

    count = kprintu(value.sections.lo, 16, fg_color, bg_color);
    gotoxy(save_h, save_v);
    for (int c = 8 - count; c > 0; c--)
    {
        kprintc('0', fg_color, bg_color);
    }
    kprintu(value.sections.lo, 16, fg_color, bg_color);
}


void clear_screen()
{
    struct TextCell *temp = (struct TextCell *)0xb8000;
    for(int i = 0; i < MAX_TEXT; i++)
    {
        temp->ch = ' ';
        text_cursor->attribute.fg_color  = GRAY;
        text_cursor->attribute.bg_color  = BLACK;
        temp++;
    }
    text_cursor = (struct TextCell *)0xb8000;
    currh = 0;
    currv = 0;
}


void pad_pointer(uint32_t p)
{
    if (p == 0)
    {
        kprints("0000000", current_default_foreground, current_default_background);
    }
    else if (p < 0x10000000)
    {
        uint32_t mask = 0xf0000000;
        while ((p & mask) == 0)
        {
            kprintc('0', current_default_foreground, current_default_background);
            mask >>= 4;
        }
    }
}


void kprintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    for (const char* p = format; *p != '\0'; ++p)
    {
        switch(*p)
        {
            case '%':
                switch(*++p) // read format symbol
                {
                    case 'c':
                        kprintc((char) va_arg(args, int), current_default_foreground, current_default_background);
                        continue;
                   case 's':
                        kprints(va_arg(args, char*), current_default_foreground, current_default_background);
                        continue;
                    case 'd':
                        kprinti(va_arg(args, int32_t), 10, current_default_foreground, current_default_background);
                        continue;
                    case 'u':
                        kprintu(va_arg(args, uint32_t), 10, current_default_foreground, current_default_background);
                        continue;
                    case 'x':
                        kprintu(va_arg(args, uint32_t), 16, current_default_foreground, current_default_background);
                        continue;
                    case 'l':
                        kprintlx(va_arg(args, uint64_t), current_default_foreground, current_default_background);
                        continue;
                    case 'p':
                        uint32_t ptr = va_arg(args, uint32_t);
                        pad_pointer(ptr);
                        kprintu(ptr, 16, current_default_foreground, current_default_background);
                        continue;
                    
                    case '%':
                        kprintc('%', current_default_foreground, current_default_background);
                        continue;
                }
        }
        kprintc(*p, current_default_foreground, current_default_background);
    }
}