#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "acpi.h"
#include "kernel.h"
#include "terminal.h"


struct RSDP_Extended_Descriptor *rsdp;


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
    int8_t* c_ptr = (int8_t *) candidate;

    for (i = 0; i < sizeof(struct RSDP_Descriptor); i++)
    {
        checksum += c_ptr[i];
    }

    if ((checksum & 0x000000ff) != 0)
    {
        kprints("First checksum failed!\n", GRAY, BLACK);
        return false;
    }
    kprints("First checksum good... ", GRAY, BLACK);

    if (candidate->ver_1.Revision == 0)
    {
        kprints("ACPI 1.0, no extended checksum.\n", GRAY, BLACK);
        return true;
    }
    else if (candidate->ver_1.Revision != 2)
    {
        kprints("Invalid revision, RSDP corrupted.\n", GRAY, BLACK);
        return false;
    }

    //note that checksum and i are *not* reset!
    for (; i < sizeof(struct RSDP_Extended_Descriptor); i++)
    {
        checksum += c_ptr[i];
    }

    if ((checksum & 0x000000ff) != 0)
    {
        kprints("second checksum failed!\n", GRAY, BLACK);
        return false;
    }

    kprints("second checksum good.\n", GRAY, BLACK);
    return true;
}


void init_acpi()
{
    kprints("\nSeeking RSDP... ", GRAY, BLACK);

    rsdp = scan_rsdp_signature();
    kprints("RDSP candidate found\n", GRAY, BLACK);

    kprints("OEM ID: ", GRAY, BLACK);
    for (unsigned int i = 0; i < 6; i++)
    {
        kprintc(rsdp->ver_1.OEM_ID[i], BLACK, WHITE);
    }
    kprintc('\n', GRAY, BLACK);


    if (!validate_rsdp_checksum(rsdp))
    {
        kprints("Error: invalid RDSP", WHITE, BLACK);
        panic();
    }
}