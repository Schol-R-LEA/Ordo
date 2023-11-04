#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>

void disable_legacy_timer(void);
void init_timer(void);
void disable_timer(void);
void enable_timer(void);
bool timer_enabled(void);

#endif