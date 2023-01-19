#ifndef PORT_IO_H
#define PORT_IO_H

// inX and outX functions based on those found at
// https://wiki.osdev.org/Inline_Assembly/Examples


static inline uint8_t inb8(uint8_t port)
{
    uint8_t ret;
    __asm__ __volatile__ ("inb %1, %0"
                          : "=a"(ret)
                          : "Ni"(port) );
    return ret;
}


static inline uint8_t inb16(uint16_t port)
{
    uint8_t ret;
    __asm__ __volatile__ ("inb %1, %0"
                          : "=a"(ret)
                          : "Nd"(port) );
    return ret;
}

static inline void outb8(uint8_t port, uint8_t val)
{
    __asm__ __volatile__ ("outb %0, %1"
                          :
                          : "a"(val), "Ni"(port) );
}


static inline void outb16(uint16_t port, uint8_t val)
{
    __asm__ __volatile__ ("outb %0, %1"
                          :
                          : "a"(val), "Nd"(port) );
}

#endif