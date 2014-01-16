#include "LPC11xx.h"
#include "xprintf.h"
#include "display_timer.h"
#include "display.h"
#include "ping.h"
#define _BV(x) (1 << (x))

short *display_data;


void display_timer_init(void)
{
    matrix_init();
    display_data = DotPicture[1];
	LPC_SYSCON->SYSAHBCLKCTRL |=  (1 << 10);
    LPC_TMR32B1 -> PR = (3*32)-1;			
    LPC_TMR32B1 -> MR3 = 10-1;			
    LPC_TMR32B1 -> MCR &= ~(7 << 9);	
    LPC_TMR32B1 -> MCR |=  (3 << 9);	
    NVIC -> ISER[0] |= (1<<19);
    NVIC_SetPriority(19,4);

    LPC_TMR32B1->TCR = 1;
}

void CT32B1_IRQHandler(void)
{
    display(display_data);
	LPC_TMR32B1->IR =0x8;
    
}
