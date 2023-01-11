#ifndef GDT_H
#define GDT_H

enum GDT_entry: uint16_t
{
    null_selector,
    system_code_selector = 1 << 3,
    system_data_entry = 2 << 3,
    system_tss_entry = 3 << 3,
    user0_code_entry = (4 << 3) + 3,
    user0_data_entry = (5 << 3) + 3
};

#endif