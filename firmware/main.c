#include "LPC11xx.h"		/* LPC11xx register definitions */
#include "flash_nvol.h"
#include "myinit.h"
#include "uart.h"
#include "ina226.h"
#include "xprintf.h"


#define _BV(x) (1 << (x))
#define TIMEOUT  500



void ioinit(void){
    LPC_IOCON -> PIO0_4 = 0;
    LPC_IOCON -> PIO0_5 = 0;
    LPC_IOCON -> PIO0_6 = 0;
    LPC_IOCON -> PIO0_7 = 0;
    LPC_IOCON -> PIO0_8 = 0;
    LPC_IOCON -> PIO0_9 = 0;
    LPC_IOCON -> SWCLK_PIO0_10 = 1;
    LPC_IOCON -> R_PIO0_11 = 1;
    LPC_IOCON -> R_PIO1_0 = 1;
    LPC_IOCON -> R_PIO1_1 = 1;
    LPC_IOCON -> R_PIO1_2 = 1;
    LPC_IOCON->SWDIO_PIO1_3 = 0x3;
    LPC_IOCON -> PIO2_2 = 0;
    LPC_IOCON -> PIO2_3 = 0;
    LPC_IOCON -> PIO2_6 = 0;
    LPC_IOCON -> PIO2_7 = 0;
    LPC_IOCON -> PIO2_8 = 0;
    LPC_IOCON -> PIO2_10 = 0;

 
    LPC_GPIO0 -> DIR |= ( _BV(4) | _BV(5) | _BV(6) | _BV(7) | _BV(8) | _BV(9) | _BV(10) | _BV(11));
    LPC_GPIO0 -> DATA &= ~(( _BV(4) | _BV(5) | _BV(6) | _BV(7) | _BV(8) | _BV(9) | _BV(10) | _BV(11)));
 
    LPC_GPIO1 -> DIR = ( _BV(0) | _BV(1) | _BV(2));
    LPC_GPIO1 -> DATA = ( _BV(0) | _BV(1) | _BV(2));
 

    LPC_GPIO2 -> DIR |= _BV(2) | _BV(3) |_BV(6)| _BV(7) | _BV(8) | _BV(10);
    LPC_GPIO2 -> DATA &= ~(_BV(2) | _BV(7) | _BV(8) | _BV(10)); 


}


int main (void)
{
    int data = 0;
    MySystemInit();
    NVOL_Init();

    ioinit();
    uart_init(38400);
    xdev_in(uart_getc);
    xdev_out(uart_putc);
    /* Enable SysTick timer in interval of 1ms */
    SysTick->LOAD = AHB_CLOCK / 1000 - 1;
    SysTick->CTRL = 0x07;

    
    while (1) {
    }
    return 0;
}


void SysTick_Handler (void)		/* 1kHz Timer ISR */
{
    static int i = 0;
    SysTick->CTRL;	
	/* Clear overflow flag (by reading COUNTFLAG)*/ 
    i++;
    xprintf("test,%d\n",i);
    
}

