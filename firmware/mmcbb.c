/*------------------------------------------------------------------------/
/  Bitbanging MMCv3/SDv1/SDv2 (in SPI mode) control module for PFF
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2010, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/--------------------------------------------------------------------------/
 Features:

 * Very Easy to Port
   It uses only 4-6 bit of GPIO port. No interrupt, no SPI port is used.

 * Platform Independent
   You need to modify only a few macros to control GPIO ports.

/-------------------------------------------------------------------------*/


#include "diskio.h"
#include "LPC11xx.h"

#include "xprintf.h"
#include "spi_lpc.h"
#include "uart.h"
/*-------------------------------------------------------------------------*/
/* Platform dependent macros and functions needed to be modified           */
/*-------------------------------------------------------------------------*/

#define	INIT_PORT()	power_on()	/* Initialize MMC control port (CS/CLK/DI:output, DO:input) */
#define DLY_US(n)	dly_us(n)	/* Delay n microseconds */
#define	FORWARD(d)	uart_putc(d)	/* Data in-time processing function (depends on the project) */

#define _BV(x) (1 << (x))

#define	CS_L() LPC_GPIO0 -> DATA &= ~(1<<10)/* Set MMC CS "high" */
#define CS_H() LPC_GPIO0 -> DATA |= (1<<10);/* Set MMC CS "low" */
#define	CK_L() LPC_GPIO0 -> DATA &= ~(1<<6)/* Set MMC CS "high" */
#define CK_H() LPC_GPIO0 -> DATA |= (1<<6);/* Set MMC CS "low" */
#define	DI_L() LPC_GPIO0 -> DATA &= ~(1<<9)/* Set MMC CS "high" */
#define DI_H() LPC_GPIO0 -> DATA |= (1<<9);/* Set MMC CS "low" */

#define DO 	 LPC_GPIO0 -> DATA |= (1<<9);/* Get MMC DO value (high:true, low:false) */



/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* Definitions for MMC/SDC command */
#define CMD0	(0x40+0)	/* GO_IDLE_STATE */
#define CMD1	(0x40+1)	/* SEND_OP_COND (MMC) */
#define	ACMD41	(0xC0+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(0x40+8)	/* SEND_IF_COND */
#define CMD16	(0x40+16)	/* SET_BLOCKLEN */
#define CMD17	(0x40+17)	/* READ_SINGLE_BLOCK */
#define CMD24	(0x40+24)	/* WRITE_BLOCK */
#define CMD55	(0x40+55)	/* APP_CMD */
#define CMD58	(0x40+58)	/* READ_OCR */

/* Card type flags (CardType) */
#define CT_MMC				0x01	/* MMC ver 3 */
#define CT_SD1				0x02	/* SD ver 1 */
#define CT_SD2				0x04	/* SD ver 2 */
#define CT_SDC				(CT_SD1|CT_SD2)	/* SD */
#define CT_BLOCK			0x08	/* Block addressing */



static
BYTE CardType;			/* b0:MMC, b1:SDv1, b2:SDv2, b3:Block addressing */

static inline void dly_us(int d)
 {
   /* timer ISRの中で30us待機する。 */
     int32_t waituntil = (int32_t)SysTick->VAL - (AHB_CLOCK * d / 1000000); /* 30us後のカウント値を計算 */
     while (waituntil < (int32_t)SysTick->VAL); /* ダウンカウンタが所定の値まで減少するのを待つ。 */
 }

static void power_on (void)
{
    LPC_IOCON -> SWCLK_PIO0_10 = (1<<0)|(1<<0);
	LPC_GPIO0 -> DIR |= _BV(10);		/* CS# (P0_2) = output */
	CS_H();
	/* Initialize SPI0 module and attach it to the I/O pad */
    LPC_SYSCON -> SYSAHBCLKCTRL |= (0x1<<11);
	LPC_SYSCON -> PRESETCTRL &= ~_BV(0);	/* Set LPC_SSP0 ->  reset */
	LPC_SYSCON -> PRESETCTRL |= _BV(0);	/* Release LPC_SSP0 ->  reset */
    LPC_SYSCON -> SSP0CLKDIV = 1;	 
	LPC_SSP0 -> CPSR = 0x02;		/* fc=PCLK/2 */
	LPC_SSP0 -> CR0 = 0x0007;		/* Mode-0, 8-bit */
	LPC_SSP0 -> CR1 = 0x02;			/* Enable SPI */
	LPC_IOCON -> SCK_LOC = 0x02;	/* SCK0 location = PIO0_6 */
	LPC_IOCON -> PIO0_6 = 0x02;	/* SCK0 */
	LPC_IOCON -> PIO0_9 = 0x01;	/* MOSI0 */
	LPC_IOCON -> PIO0_8 = 0x11;	/* MISO0/pull-up */
    
    ssp_init();
}


static void xmit_mmc (BYTE d)
{
    char dummy;
    uint8_t  r_buf = 0;
    while ((LPC_SSP0 -> SR & ((1<<1)|(1<<4))) != (1<<1));

	LPC_SSP0 -> DR = d;		/* Start an SPI transaction */
	while (!(LPC_SSP0 -> SR & _BV(2))) ;	/* Wait for the end of transaction */
	dummy = LPC_SSP0 -> DR;	/* Return received byte */
    return ;
}

static BYTE rcvr_mmc (void)
{
    BYTE r_buf = 0;
    

	LPC_SSP0 -> DR = 0xff;		/* Start an SPI transaction */
	while (!(LPC_SSP0 -> SR & _BV(2))) ;	/* Wait for the end of transaction */
	r_buf = LPC_SSP0 -> DR;	/* Return received byte */
    return r_buf;
}

static void skip_mmc (WORD n)   
{
    char dummy;
    do {
        while ((LPC_SSP0 -> SR & ((1<<1)|(1<<4))) != (1<<1));
        LPC_SSP0 -> DR = 0xff;		/* Start an SPI transaction */
        while (!(LPC_SSP0 -> SR & _BV(2))) ;	/* Wait for the end of transaction */
        dummy = LPC_SSP0 -> DR;	/* Return received byte */
    }while(--n);
	
    return;
    
}



/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static
void release_spi (void)
{
	CS_H();
	rcvr_mmc();
}


/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

static
BYTE send_cmd (
	BYTE cmd,		/* Command byte */
	DWORD arg		/* Argument */
)
{
	BYTE n, res;


	if (cmd & 0x80) {	/* ACMD<n> is the command sequense of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1) return res;
	}
	/* Select the card */
	CS_H(); rcvr_mmc();
	CS_L(); rcvr_mmc();
    
	/* Send a command packet */
	xmit_mmc(cmd);					/* Start + Command index */
	xmit_mmc((BYTE)(arg >> 24));	/* Argument[31..24] */
	xmit_mmc((BYTE)(arg >> 16));	/* Argument[23..16] */
	xmit_mmc((BYTE)(arg >> 8));		/* Argument[15..8] */
	xmit_mmc((BYTE)arg);			/* Argument[7..0] */
	n = 0x01;						/* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95;		/* Valid CRC for CMD0(0) */
	if (cmd == CMD8) n = 0x87;		/* Valid CRC for CMD8(0x1AA) */
	xmit_mmc(n);

	/* Receive a command response */
	n = 10;								/* Wait for a valid response in timeout of 10 attempts */
	do {
		res = rcvr_mmc();
	} while ((res & 0x80) && --n);

	return res;			/* Return with the response value */
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (void)
{
	BYTE n, cmd, ty, buf[4];
	UINT tmr;


	INIT_PORT();

	CS_H();
	skip_mmc(10);			/* Dummy clocks */

        
	ty = 0;
	if (send_cmd(CMD0, 0) == 1) {			/* Enter Idle state */
		if (send_cmd(CMD8, 0x1AA) == 1) {	/* SDv2 */

			for (n = 0; n < 4; n++) buf[n] = rcvr_mmc();	/* Get trailing return value of R7 resp */

			if (buf[2] == 0x01 && buf[3] == 0xAA) {			/* The card can work at vdd range of 2.7-3.6V */
				for (tmr = 1000; tmr; tmr--) {				/* Wait for leaving idle state (ACMD41 with HCS bit) */
					if (send_cmd(ACMD41, 1UL << 30) == 0) break;
					DLY_US(1000);
				}
           
				if (tmr && send_cmd(CMD58, 0) == 0) {		/* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++) buf[n] = rcvr_mmc();
					ty = (buf[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;	/* SDv2 (HC or SC) */
                
				}
			}
        
		} else {							/* SDv1 or MMCv3 */
			if (send_cmd(ACMD41, 0) <= 1) 	{
  				ty = CT_SD1; cmd = ACMD41;	/* SDv1 */
			} else {
				ty = CT_MMC; cmd = CMD1;	/* MMCv3 */
			}
              xprintf("test start2\n");

			for (tmr = 1000; tmr; tmr--) {			/* Wait for leaving idle state */
				if (send_cmd(cmd, 0) == 0) break;
				DLY_US(1000);
			}
			if (!tmr || send_cmd(CMD16, 512) != 0)			/* Set R/W block length to 512 */
				ty = 0;
            xprintf("test fin2\n");

		}
        xprintf("test fin3\n");          
	}
	CardType = ty;
	release_spi();

	return ty ? 0 : STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read partial sector                                                   */
/*-----------------------------------------------------------------------*/

DRESULT disk_readp (
	BYTE *buff,		/* Pointer to the read buffer (NULL:Read bytes are forwarded to the stream) */
	DWORD lba,		/* Sector number (LBA) */
	WORD ofs,		/* Byte offset to read from (0..511) */
	WORD cnt		/* Number of bytes to read (ofs + cnt mus be <= 512) */
)
{
	DRESULT res;
	BYTE d;
	WORD bc, tmr;


	if (!(CardType & CT_BLOCK)) lba *= 512;		/* Convert to byte address if needed */

	res = RES_ERROR;
	if (send_cmd(CMD17, lba) == 0) {		/* READ_SINGLE_BLOCK */

		tmr = 1000;
		do {							/* Wait for data packet in timeout of 100ms */
			DLY_US(100);
			d = rcvr_mmc();
		} while (d == 0xFF && --tmr);

		if (d == 0xFE) {				/* A data packet arrived */
			bc = 514 - ofs - cnt;

			/* Skip leading bytes */
			if (ofs) skip_mmc(ofs);

			/* Receive a part of the sector */
			if (buff) {	/* Store data to the memory */
				do
					*buff++ = rcvr_mmc();
				while (--cnt);
			} else {	/* Forward data to the outgoing stream */
				do {
					d = rcvr_mmc();
					FORWARD(d);
				} while (--cnt);
			}

			/* Skip trailing bytes and CRC */
			skip_mmc(bc);

			res = RES_OK;
		}
	}

	release_spi();

	return res;
}



/*-----------------------------------------------------------------------*/
/* Write partial sector                                                  */
/*-----------------------------------------------------------------------*/
#if _USE_WRITE

DRESULT disk_writep (
	const BYTE *buff,	/* Pointer to the bytes to be written (NULL:Initiate/Finalize sector write) */
	DWORD sa			/* Number of bytes to send, Sector number (LBA) or zero */
)
{
	DRESULT res;
	WORD bc, tmr;
	static WORD wc;


	res = RES_ERROR;

	if (buff) {		/* Send data bytes */
		bc = (WORD)sa;
		while (bc && wc) {		/* Send data bytes to the card */
			xmit_mmc(*buff++);
			wc--; bc--;
		}
		res = RES_OK;
	} else {
		if (sa) {	/* Initiate sector write process */
			if (!(CardType & CT_BLOCK)) sa *= 512;	/* Convert to byte address if needed */
			if (send_cmd(CMD24, sa) == 0) {			/* WRITE_SINGLE_BLOCK */
				xmit_mmc(0xFF); xmit_mmc(0xFE);		/* Data block header */
				wc = 512;							/* Set byte counter */
				res = RES_OK;
			}
		} else {	/* Finalize sector write process */
			bc = wc + 2;
			while (bc--) xmit_mmc(0);	/* Fill left bytes and CRC with zeros */
			if ((rcvr_mmc() & 0x1F) == 0x05) {	/* Receive data resp and wait for end of write process in timeout of 300ms */
				for (tmr = 10000; rcvr_mmc() != 0xFF && tmr; tmr--)	/* Wait for ready (max 1000ms) */
					DLY_US(100);
				if (tmr) res = RES_OK;
			}
			release_spi();
		}
	}

	return res;
}
#endif
