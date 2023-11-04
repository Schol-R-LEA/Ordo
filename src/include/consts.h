#ifndef CONSTS_H
#define CONSTS_H

#include <stddef.h>
#include <stdint.h>
#include "consts.h"
#include "paging.h"
#include "gdt.h"
#include "tss.h"
#include "idt.h"


#define page_directory_offset 0x000400000
#define page_tables_offset (page_directory_offset + PD_SIZE)
#define gdt_physical_offset (page_tables_offset + (16 + MBYTE))
#define tss_physical_offset (gdt_physical_offset + 0x00010000)
#define idt_physical_offset (tss_physical_offset + 0x00001000)
#define stack_physical_offset (idt_physical_offset + 0x00001000)
#define heap_physical_offset (stack_physical_offset + 0x00004000)


// addresses imported from linker script
// all of these except for kernel_physical_base and kernel_physical_end
// point to locations in the higher half
extern const uint8_t *kernel_physical_base, *kernel_physical_end,
                     *kernel_base, *kernel_end;
extern const union Page_Table_Entry *page_tables;
extern const union Page_Directory_Entry *page_directory;
extern const union GDT_Entry *gdt;
extern const struct TSS *default_tss;
extern const struct Interrupt_Descriptor_32 *idt;
extern const size_t *kernel_stack;
extern const struct Free_List_Entry *heap;


// computed address for the locations in physical memory 
extern const struct kdata *kernel_boot_data_physical_base;
extern const union GDT_Entry *gdt_physical_base;
extern const struct TSS *tss_physical_base;
extern const struct Interrupt_Descriptor_32 *idt_physical_base;
extern const size_t *kernel_stack_physical_base;
extern const struct Free_List_Entry *heap_physical_base;


extern const size_t kernel_effective_size, kernel_boot_data_physical_size,
                    page_tables_size, page_directory_size,
                    gdt_physical_size, tss_physical_size, idt_physical_size,
                    pmmap_physical_size,
                    kernel_stack_physical_size,
                    system_reserved_size;


#endif