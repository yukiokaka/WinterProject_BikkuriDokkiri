#include "LPC11xx.h"		/* LPC11xx register definitions */
#include "flash_nvol.h"
#include "myinit.h"
#include "uart.h"
#include "xprintf.h"
#include "diskio.h"
#include "pff.h"
#include "rtc.h"
#include "spi_lpc.h"
#include "display_timer.h"
#include "ircomm.h"
#include "ping.h"
#include "pccomm.h"
#include "timer_controller.h"
#include "transmit_display_data.h"
#include "main.h"

#define _BV(x) (1 << (x))


/*---------------------------------------------------------*/
/* User Provided Timer Function for FatFs module           */
/*---------------------------------------------------------*/

DWORD get_fattime (void)
{
	return	  ((DWORD)(2010 - 1980) << 25)	
        | ((DWORD)1 << 21)
        | ((DWORD)1 << 16)
        | ((DWORD)0 << 11)
        | ((DWORD)0 << 5)
        | ((DWORD)0 >> 1);
}


void die(FRESULT rc)
{
    xprintf("Failed with rc=%u.\n", rc);
	for (;;) ;
}

/*---------------------------------------------------------*/

/* 1kHz Timer ISR */
void SysTick_Handler (void)
{
    static int i = 0;
    SysTick->CTRL;	   

    i++;
    
}


int Mode = HOST_MODE;

int main (void)
{

    MySystemInit();
    NVOL_Init();

    ioinit();
    uart_init(38400);
    xdev_in(uart_getc);
    xdev_out(uart_putc);
    ircomm_init();
    display_timer_init();
    ping_init();

    /* Enable SysTick timer in interval of 1ms */
    SysTick->LOAD = AHB_CLOCK  - 1;
    SysTick->CTRL = 0x07;
    
    unsigned long data = 0;
    while(1) { 
        data = 0;
        enable_timer();
        pc_state_machine(); 
        if(Mode == HOST_MODE) {
            data = ircomm_recv();
            if(data == CMD_PING) {   
                pong();
                send_display_data();
            }

        }
        else if(Mode == SLAVE_MODE) {
            data = ircomm_recv();
            if(data == CMD_PONG) {
                recv_display_data();
            }

        }
    }
    return 0;
}


