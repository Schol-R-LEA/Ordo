#ifndef GDT_H
#define GDT_H

enum GDT_entry
{
    null_entry,
    system_code_entry,
    system_data_entry,
    system_tss_entry,
    user0_code_entry,
    user0_data_entry
};

#endif