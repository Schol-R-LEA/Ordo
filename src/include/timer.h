#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>

void init_timer(void);
void diasble_timer(void);
void enable_timer(void);
bool timer_enabled(void);

#endif