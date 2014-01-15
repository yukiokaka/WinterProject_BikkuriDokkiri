#include "LPC11xx.h"		/* LPC11xx register definitions */
#include "flash_nvol.h"
#include "myinit.h"
#include "uart.h"
#include "xprintf.h"
#include "diskio.h"
#include "pff.h"
#include "rtc.h"
#include "spi_lpc.h"
#include "display.h"
#define _BV(x) (1 << (x))
#define TIMEOUT  500

/*---------------------------------------------------------*/
/* User Provided Timer Function for FatFs module           */
/*---------------------------------------------------------*/

DWORD get_fattime (void)
{
	return	  ((DWORD)(2010 - 1980) << 25)	/* Fixed to Jan. 1, 2010 */
        | ((DWORD)1 << 21)
        | ((DWORD)1 << 16)
        | ((DWORD)0 << 11)
        | ((DWORD)0 << 5)
        | ((DWORD)0 >> 1);
}


void die (		/* Stop with dying message */
          FRESULT rc	/* FatFs return value */
                )
{
	xprintf("Failed with rc=%u.\n", rc);
	for (;;) ;
}


void ioinit(void){
    LPC_IOCON -> PIO0_4 = 0;
    LPC_IOCON -> PIO0_5 = 0;
    LPC_IOCON -> PIO0_6 = 0;
    LPC_IOCON -> PIO0_7 = 0;
    LPC_IOCON -> PIO0_8 = 0;
    LPC_IOCON -> PIO0_9 = 0;
    LPC_IOCON -> SWCLK_PIO0_10 = 1;
    LPC_IOCON -> R_PIO0_11 = (1 << 7) | (1 << 0);
    LPC_IOCON -> R_PIO1_0 = (1 << 7) | (1 << 0);
    LPC_IOCON -> R_PIO1_1 = (1 << 7) | (1 << 0);
    LPC_IOCON -> R_PIO1_2 = (1 << 7) | (1 << 0) ;
    LPC_IOCON-> SWDIO_PIO1_3 = (1 << 7) |(1 << 0) ;
    LPC_IOCON-> PIO1_4 = (1 << 7) | 0;
    LPC_IOCON-> PIO1_5 = 0x0;
    LPC_IOCON-> PIO1_8 = 0x0;
    
    LPC_IOCON -> PIO2_2 = 0;
    LPC_IOCON -> PIO2_3 = 0;
    LPC_IOCON -> PIO2_6 = 0;
    LPC_IOCON -> PIO2_7 = 0;
    LPC_IOCON -> PIO2_8 = 0;
    LPC_IOCON -> PIO2_10 = 0;

 
    LPC_GPIO0 -> DIR |=  _BV(4) | _BV(5) | _BV(6) | _BV(7) | _BV(8) | _BV(9) | _BV(10) ;
    LPC_GPIO0 -> DATA &= ~( _BV(4) | _BV(5) | _BV(6) | _BV(7) | _BV(8) | _BV(9) | _BV(10));
 
    LPC_GPIO1 -> DIR =  _BV(3) | _BV(4) | _BV(5) | _BV(8);
    LPC_GPIO1 -> DATA = 0;
 

    LPC_GPIO2 -> DIR |= _BV(2) | _BV(3) |_BV(6)| _BV(7) | _BV(8) | _BV(10);
    LPC_GPIO2 -> DATA &= ~(_BV(2) | _BV(7) | _BV(8) | _BV(10)); 


}

volatile char device_num = 1;
void ping(void)
{
    unsigned char data =0xc0;
    ircomm_send(&data);
    data =device_num;
    ircomm_send(&data);
    
}

int main (void)
{
	FATFS fatfs;			/* File system object */
	DIR dir;				/* Directory object */
	FILINFO fno;			/* File information object */
	WORD bw, br, i;
	BYTE buff[64];
    
    unsigned data = 0;
    
    MySystemInit();
    NVOL_Init();

    ioinit();
    uart_init(38400);
    xdev_in(uart_getc);
    xdev_out(uart_putc);
    ircomm_init();
    matrix_init();
    
    /* Enable SysTick timer in interval of 1ms */
    SysTick->LOAD = AHB_CLOCK  - 1;
    SysTick->CTRL = 0x07;
    
    char array[256] = {0xff};
    int test;
    for (test = 0; test < 256; test++) {
         array[test]= 0x0;
    }

    
    while(1) { 
        display(array);
        data = ircomm_recv(0);
        //ping();
    }
    
    /*SD card RW test*/
	xprintf("\nMount a volume.\n");
	FRESULT rc = pf_mount(&fatfs);
    xprintf("\nMount a volume finish .\n", rc);
	if (rc) die(rc);

	xprintf("\nOpen a test file (message.txt).\n");
	rc = pf_open("MESSAGE.TXT");
	if (rc) die(rc);

	xprintf("\nType the file content.\n");
	for (;;) {
		rc = pf_read(buff, sizeof(buff), &br);	/* Read a chunk of file */
		if (rc || !br) break;			/* Error or end of file */
		for (i = 0; i < br; i++)		/* Type the data */
			uart_putc(buff[i]);
	}
	if (rc) die(rc);

#if _USE_WRITE
	xprintf("\nOpen a file to write (write.txt).\n");
	rc = pf_open("WRITE.TXT");
	if (rc) die(rc);

	xprintf("\nWrite a text data. (Hello world!)\n");
	for (;;) {
		rc = pf_write("Hello world!\r\n", 14, &bw);
		if (rc || !bw) break;
	}
	if (rc) die(rc);

	xprintf("\nTerminate the file write process.\n");
	rc = pf_write(0, 0, &bw);
	if (rc) die(rc);
#endif

#if _USE_DIR
	xprintf("\nOpen root directory.\n");
	rc = pf_opendir(&dir, "");
	if (rc) die(rc);

	xprintf("\nDirectory listing...\n");
	for (;;) {
		rc = pf_readdir(&dir, &fno);	/* Read a directory item */
		if (rc || !fno.fname[0]) break;	/* Error or end of dir */
		if (fno.fattrib & AM_DIR)
			xprintf("   <dir>  %s\n", fno.fname);
		else
			xprintf("%8lu  %s\n", fno.fsize, fno.fname);
	}
	if (rc) die(rc);
#endif

	xprintf("\nTest completed.\n");
	xprintf("start .\n");

    return 0;
}


void SysTick_Handler (void)		/* 1kHz Timer ISR */
{
    static int i = 0;
    SysTick->CTRL;	
	/* Clear overflow flag (by reading COUNTFLAG)*/ 
    i++;
    
}

