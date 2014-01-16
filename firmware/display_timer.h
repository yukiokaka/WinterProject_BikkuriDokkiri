#ifndef DISPLAY_TIMER_H_
#define DISPLAY_TIMER_H_

#include "display.h"

extern short *display_data;

#define  disable_display_timer() LPC_TMR32B1->TCR = 0
#define  enable_display_timer() LPC_TMR32B1->TCR = 1



void display_timer_init(void);
void CT32B1_IRQHandler(void);

#endif
