#ifndef ACPI_H
#define ACPI_H



#define HiBIOS_Region 0x000E0000
#define HiBIOS_End    0x000FFFFF



struct RSDP_Descriptor {
    char Signature[8];
    uint8_t Checksum;
    char OEM_ID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;
} __attribute__ ((packed));



struct RSDP_Extended_Descriptor
{
    struct RSDP_Descriptor ver_1;

    uint32_t Length;
    uint64_t XsdtAddress;
    uint8_t ExtendedChecksum;
    uint8_t reserved[3];
} __attribute__ ((packed));



void init_acpi();


#endif