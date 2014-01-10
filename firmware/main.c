/*-------------------------------------------------------------------------/
/
/  Copyright (C) 2011, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/

#include <string.h>
#include "LPC1100.h"
#include "iic.h"
#include "xprintf.h"
#include "uart.h"
#include "diskio.h"
#include "ff.h"
#include "rtc.h"
#include "terminal.h"
#define F_CPU	36000000

#define _MODE_STANDALONE	1	/* Commanded by 1:Motion/UART, 0:UART only */




/*--------------------------------------------------------------*/
/* Initialization and main processing loop                      */
/*--------------------------------------------------------------*/

int main (void)
{


	/* Enable SysTick timer in interval of 1 ms */
	SYST_RVR = F_CPU / 1000 - 1;
	SYST_CSR = 0x07;

	/* Initialize I2C module */
    i2c0_init();
 
    /* Initialize UART and attach it to xprintf module for console */
    uart0_init();
    xdev_out(uart0_putc);
    xdev_in(uart0_getc);
    xputs("MARY-MB/OB/SR test monitor\n");

    /* Initialize OLED module */


    terminal_init();
    
    for (;;) {
        terminal();
    }
}
