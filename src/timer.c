#include <stdbool.h>
#include "timer.h"
#include "port_io.h"

void init_timer()
{
    // first, disable the 8259A PIC
    outb(0xa1, 0xff);
    outb(0x21, 0xff);

    
}

void diasble_timer()
{

}

void enable_timer()
{

}

bool timer_enabled()
{
    return true;
}