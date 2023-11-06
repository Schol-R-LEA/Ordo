#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>

void disable_legacy_timer();
void init_timer();
void disable_timer();
void enable_timer();
bool timer_enabled();

#endif