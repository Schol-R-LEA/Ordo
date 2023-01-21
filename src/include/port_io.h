#ifndef PORT_IO_H
#define PORT_IO_H

// inX and outX functions based on those found at
// https://wiki.osdev.org/Inline_Assembly/Examples


static inline uint8_t inb8(uint8_t port)
{
    uint8_t ret;
    __asm__ __volatile__ ("inb %1, %0"
                          : "=a"(ret)
                          : "N"(port) );
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
                          : "a"(val), "N"(port) );
}


static inline void outb16(uint16_t port, uint8_t val)
{
    __asm__ __volatile__ ("outb %0, %1"
                          :
                          : "a"(val), "Nd"(port) );
}


static inline uint16_t inw8(uint8_t port)
{
    uint16_t ret;
    __asm__ __volatile__ ("inw %1, %0"
                          : "=a"(ret)
                          : "N"(port) );
    return ret;
}


static inline uint16_t inw16(uint16_t port)
{
    uint16_t ret;
    __asm__ __volatile__ ("inw %1, %0"
                          : "=a"(ret)
                          : "Nd"(port) );
    return ret;
}


static inline void outw8(uint8_t port, uint16_t val)
{
    __asm__ __volatile__ ("outw %0, %1"
                          :
                          : "a"(val), "N"(port) );
}


static inline void outw16(uint16_t port, uint16_t val)
{
    __asm__ __volatile__ ("outw %0, %1"
                          :
                          : "a"(val), "Nd"(port) );
}



static inline uint32_t inl8(uint8_t port)
{
    uint32_t ret;
    __asm__ __volatile__ ("inl %1, %0"
                          : "=a"(ret)
                          : "N"(port) );
    return ret;
}


static inline uint32_t inl16(uint16_t port)
{
    uint32_t ret;
    __asm__ __volatile__ ("inl %1, %0"
                          : "=a"(ret)
                          : "Nd"(port) );
    return ret;
}


static inline void outl8(uint8_t port, uint32_t val)
{
    __asm__ __volatile__ ("outl %0, %1"
                          :
                          : "a"(val), "N"(port) );
}


static inline void outl16(uint16_t port, uint32_t val)
{
    __asm__ __volatile__ ("outl %0, %1"
                          :
                          : "a"(val), "Nd"(port) );
}

#endif