#include <stddef.h>
#include <stdint.h>
#include "consts.h"
#include "kernel.h"
#include "gdt.h"
#include "idt.h"
#include "mem.h"


// provision 3M for the kernel, regardless of its actual size
const size_t kernel_effective_size = 0x00300000;

const size_t kernel_boot_data_physical_size = sizeof(struct kdata);

const size_t page_directory_size = PD_SIZE;
const size_t page_tables_size = (16 * MBYTE);

const size_t gdt_physical_size = 0x00010000;
const size_t tss_physical_size = 0x00001000;      //provide ample space for the TSS data structure
const size_t idt_physical_size = sizeof(struct Interrupt_Descriptor_32) * IDT_SIZE;


// kernel stack is set to 16KiB
const size_t kernel_stack_physical_size = 0x00004000;


// data collected by the boot loader, at an offset from the top of the HMA

const struct kdata *kernel_boot_data_physical_base = (struct kdata *) (KDATA_OFFSET - sizeof(struct kdata) - 16);

const union Page_Table_Entry *page_tables = (union Page_Table_Entry *) page_tables_offset;
const union Page_Directory_Entry *page_directory = (union Page_Directory_Entry *) page_directory_offset;

const union GDT_Entry *gdt_physical_base = (union GDT_Entry *) gdt_physical_offset;
const struct TSS *tss_physical_base = (struct TSS *) tss_physical_offset;
const struct Interrupt_Descriptor_32 *idt_physical_base = (struct Interrupt_Descriptor_32 *) idt_physical_offset;

const size_t *kernel_stack_physical_base = (size_t *) stack_physical_offset;

const struct Free_List_Entry *heap_physical_base = (struct Free_List_Entry *) heap_physical_offset;
