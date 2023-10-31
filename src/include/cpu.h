#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <cpuid.h>
#include "cpu.h"


struct cpu_features
{
    bool apic,
         sse, sse2, sse3, ssse3, sse4, sse4_1, sse4_2,
         avx, avx2, avx256, avx512;
};

struct cpu_info
{
    uint32_t id_code;
    char vendor_sig[13];
    uint32_t max;
    struct cpu_features features;
};

extern struct cpu_info cpu_details;

void get_cpu_details();


#endif