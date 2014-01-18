#include "LPC11xx.h"
#include "xprintf.h"
#include "ping.h"
#include "ircomm.h"
#include "main.h"
#include "transmit_display_data.h"

#define _BV(x) (1 << (x))


volatile unsigned char device_num = 1;

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


int ping(void)
{
    
    static unsigned char data = 0;
    data = CMD_PING;
    
    while(ircomm_send(&data) < 0);
    return 0;
}

int pong(void)
{
    int time = 0;
    static unsigned char data = 0;
    data = CMD_PONG;
    while(ircomm_send(&data) < 0);
    
    return 0;
}

int ping_enable = 1;
int pong_enable = 0;
int send_data_flg = 0;
void CT16B1_IRQHandler(void)
{
    if(Mode == HOST_MODE) {
        if(ping_enable) {
            ping();
            send_display_data();
        }
    }
	LPC_TMR16B1->IR =0x8;    
}


