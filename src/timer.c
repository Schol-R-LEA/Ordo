#include <stdbool.h>
#include "timer.h"
#include "port_io.h"

void disable_legacy_timer(void)
{
    //disable the 8259A PIC
    outb(0xa1, 0xff);
    outb(0x21, 0xff);
}


void init_timer(void)
{
    
}



void disable_timer(void)
{

}

void enable_timer(void)
{

}

bool timer_enabled()
{
    return true;
}