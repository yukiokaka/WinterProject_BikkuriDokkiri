#include "LPC11xx.h"
#include "xprintf.h"
#include "ping.h"
#include "ircomm.h"
#define _BV(x) (1 << (x))

volatile unsigned char device_num = 0;
int ping(void)
{
    static char cnt = 0;   
    static unsigned char data = 0;
 
    
    if(ircomm_send(&data) < 0) {
        return -1;
    }

    
}

void ping_init(void)
{
	LPC_SYSCON->SYSAHBCLKCTRL |=  (1 << 8);
    
    LPC_TMR16B1->PR = (3*64)-1;        
    LPC_TMR16B1->MR3 = 62500-1;       
    LPC_TMR16B1->MCR &= ~(7UL << 9); 
    LPC_TMR16B1->MCR |=  (3UL << 9); 
    NVIC -> ISER[0] |= (1<<17);
    NVIC_SetPriority(17,3);
    LPC_TMR16B1->TCR = 1;     
}

void CT16B1_IRQHandler(void)
{
    static int cnt = 0;
    ping();
	LPC_TMR16B1->IR =0x8;
    
}
