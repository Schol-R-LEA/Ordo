#include <stdint.h>
#include <cpuid.h>
#include "cpu.h"


struct cpu_info cpu_details;


void get_cpu_details(void)
{
    unsigned int eax, ebx, ecx, edx;

    if (__get_cpuid(0, &eax, &ebx, &ecx, &edx))
    {
        cpu_details.max = eax;
        cpu_details.vendor_sig[0] = (char) (ebx & 0xff);
        cpu_details.vendor_sig[1] = (char) ((ebx >> 8) & 0xff);
        cpu_details.vendor_sig[2] = (char) ((ebx >> 16) & 0xff);
        cpu_details.vendor_sig[3] = (char) (ebx >> 24);
        cpu_details.vendor_sig[4] = (char) (edx & 0xff);
        cpu_details.vendor_sig[5] = (char) ((edx >> 8) & 0xff);
        cpu_details.vendor_sig[6] = (char) ((edx >> 16) & 0xff);
        cpu_details.vendor_sig[7] = (char) (edx >> 24);
        cpu_details.vendor_sig[8] = (char) (ecx & 0xff);
        cpu_details.vendor_sig[9] = (char) ((ecx >> 8) & 0xff);
        cpu_details.vendor_sig[10] = (char) ((ecx >> 16) & 0xff);
        cpu_details.vendor_sig[11] = (char) (ecx >> 24);
        cpu_details.vendor_sig[12] = '\0';
    }

    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx))
    {
        cpu_details.features.apic = edx & (1 << 9);
    }
}
