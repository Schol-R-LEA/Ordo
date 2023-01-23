#ifndef _TSS_H
#define _TSS_H


struct TSS
{
    uint32_t link,
             esp0, ss0, esp1, ss1, esp2, ss2,
             eip, eflags,
             eax, ecx, edx, ebx,
             esp, ebp,
             esi, edi,
             es, cs, ss, ds, fs, gs,
             ldtr_reg, iopb, ssp;
};

extern struct TSS *default_tss;


#endif