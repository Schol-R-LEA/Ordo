#ifndef KERNEL_H
#define KERNEL_H

enum PRIVILEGE_LEVEL
{
    RING_0, RING_1, RING_2, RING_3
};


static inline void panic()
{
    __asm__ ("local_loop:\n"
             "    hlt\n"
             "    jmp local_loop");
}

#endif