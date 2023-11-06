#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>


typedef atomic_flag spinlock;


static inline void spinlock_init(spinlock *lock)
{
    atomic_flag_clear_explicit(lock, memory_order_release);
}

static inline void spinlock_acquire(spinlock *lock)
{
    while( atomic_flag_test_and_set_explicit(lock, memory_order_acquire ))
    {
        __builtin_ia32_pause();
    }
}

static inline void spinlock_release(spinlock *lock)
{
    atomic_flag_clear_explicit(lock, memory_order_release);
}

#endif