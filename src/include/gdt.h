#ifndef GDT_H
#define GDT_H

#include "kernel.h"


enum GDT_entry: uint16_t
{
    null_selector,
    system_code_selector = (1 << 3) + RING_0,
    system_data_selector = (2 << 3) + RING_0,
    system_tss_selector  = (3 << 3) + RING_0,
    user0_code_selector  = (4 << 3) + RING_3,
    user0_data_selector  = (5 << 3) + RING_3
};

#endif