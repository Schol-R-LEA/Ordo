#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "acpi.h"
#include "kernel.h"
#include "terminal.h"
#include "paging.h"


struct RSDP_Extended_Descriptor *rsdp;
struct ACPI_xSDT_Header *sdt;

bool match_rsdp_signature(const char* candidate)
{
    char SIG[] = "RSD PTR ";

    for (uint8_t i = 0; i < 8; i++)
    {
        if (candidate[i] != SIG[i])
        {
            return false;
        }
    }

    return true;
}


struct RSDP_Extended_Descriptor* scan_rsdp_signature()
{
    bool found = false;
    char* curr_scan;

    for (curr_scan = (char *) HiBIOS_Region; !found && curr_scan < (char *) HiBIOS_End; curr_scan += 16)
    {
        found = match_rsdp_signature(curr_scan);
        if (found)
        {
            return ((struct RSDP_Extended_Descriptor *) curr_scan);
        }
    }

    if(!found)
    {
        kprints("Error: RSDP not found", WHITE, BLACK);
        panic();
    }
    return NULL;
}



bool validate_rsdp_checksum(struct RSDP_Extended_Descriptor *candidate)
{
    int32_t checksum = 0;
    uint8_t i;
    uint8_t* c_ptr = (uint8_t *) candidate;

    for (i = 0; i < sizeof(struct RSDP_Descriptor); i++)
    {
        checksum += c_ptr[i];
    }

    if ((checksum & 0x000000ff) != 0)
    {
        kprintf("First checksum failed!\n");
        return false;
    }
    kprintf("First checksum good... ");

    if (candidate->ver_1.Revision == 0)
    {
        kprintf("ACPI 1.0, no extended checksum.\n");
        return true;
    }
    else if (candidate->ver_1.Revision != 2)
    {
        kprintf("Invalid revision, RSDP corrupted.\n");
        return false;
    }

    //note that checksum and i are *not* reset!
    for (; i < sizeof(struct RSDP_Extended_Descriptor); i++)
    {
        checksum += c_ptr[i];
    }

    if ((checksum & 0x000000ff) != 0)
    {
        kprintf("second checksum failed!\n");
        return false;
    }

    kprintf("second checksum good.\n");
    return true;
}


bool validate_sdt_checksum(struct ACPI_xSDT_Header *candidate)
{
    uint8_t checksum = 0;
    uint8_t* c_ptr = (uint8_t *) candidate;

    for (uint8_t i = 0; i < candidate->Length; i++)
    {
        checksum += c_ptr[i];
    }

    return (checksum == 0);
}


void init_acpi()
{
    kprintf("\nSeeking RSDP... ");

    rsdp = scan_rsdp_signature();
    kprintf("RSDP candidate found\n");

    kprintf("OEM ID: ");
    for (unsigned int i = 0; i < 6; i++)
    {
        kprintc(rsdp->ver_1.OEM_ID[i], BLACK, WHITE);
    }
    kprintc('\n', GRAY, BLACK);


    if (!validate_rsdp_checksum(rsdp))
    {
        kprints("Error: invalid RSDP", LT_RED, BLACK);
        panic();
    }


    // Get the system descriptor table.
    // While the specific link used depends on the ACPI version,
    // the table structure is the same regardless of version.

    kprintf("size of the SDT header ptr: %u\n", sizeof(struct ACPI_xSDT_Header *));

    sdt = (struct ACPI_xSDT_Header *) (rsdp->ver_1.Revision == 0
                                       ? rsdp->ver_1.RsdtAddress
                                       : (uint32_t) rsdp->XsdtAddress);

    kprintf("xSDT candidate found at 0x%p, adding page to paging tables... ", sdt);

    size_t sdt_page = (size_t) sdt & PAGE_ADDRESS_MASK;
    set_page_block(sdt_page, sdt_page, 0x1000, false, false, false, false);
    kprintf("done.\n");


    for (uint8_t i = 0; i < 4; i++)
    {
        kprintc(sdt->Signature[i], WHITE, BLACK);
    }

    kprintf(" OEM ID: ");
    for (uint8_t i = 0; i < 6; i++)
    {
        kprintc(sdt->OEM_ID[i], BLACK, WHITE);
    }
    kprintc('\n', GRAY, BLACK);


    if (!validate_sdt_checksum(sdt))
    {
        kprints("Error: invalid SDT", LT_RED, BLACK);
        panic();
    }
    kprintf("SDT checksum valid.\n");
}