#ifndef TIMER_CONTROLLER_H_
#define TIMER_CONTROLLER_H_

#include "LPC11xx.h"
#include "ircomm.h"
#include "ping.h"
#include "display_timer.h"


inline void  enable_timer(void)
{
    enable_ping_timer();
    enable_ircomm_timer();
    enable_display_timer();
}


inline void disable_timer(void)
{
    disable_ping_timer();
    disable_ircomm_timer();
    disable_display_timer();
}

#endif
