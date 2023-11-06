#include <stdbool.h>
#include "timer.h"
#include "port_io.h"

void disable_legacy_timer()
{
    outb(0xa1, 0xff);
    outb(0x21, 0xff);
}


void init_timer()
{

}

void disable_timer()
{

}

void enable_timer()
{

}

bool timer_enabled()
{
    return true;
}