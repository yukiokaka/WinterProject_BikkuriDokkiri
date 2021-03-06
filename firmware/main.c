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


/* 1kHz Timer ISR */
static int i = 0;

void SysTick_Handler (void)
{
    SysTick->CTRL;

    i++;

}


int Mode = SLAVE_MODE;

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
    SysTick->LOAD = AHB_CLOCK /1000 - 1;
    SysTick->CTRL = 0x07;

    int data0 = 0, data1 = 0, data2 = 0, data3 = 0;
    int pc_state = 0;

    get_display();
    IRLEDALL_ON();
    while(1) {
        pc_state = pc_state_machine();

        if(Mode == HOST_MODE) {
            IRLED_NUM = 0;
            if(!pc_state)
                ping_enable = 1;

        }
        else if(Mode == SLAVE_MODE) {
            data0 = 0;
            data1 = 0;
            data2 = 0;
            data3 = 0;

            if(!next_ping) {
                data0 = ircomm_recv(IR0);
                if(data0 == CMD_PING) {
                    recv_display_data(IR0);
                }

            }
            else {
                IRLED_NUM = 3;
                if(!pc_state)
                    ping_enable = 1;
            }
        }
    }
    return 0;
}


