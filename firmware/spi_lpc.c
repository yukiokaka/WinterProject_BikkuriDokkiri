#include "LPC11xx.h"
#include "spi_lpc.h"


static void ssp_io_init(uint8_t prescaler){
    LPC_SYSCON -> PRESETCTRL |= (0x1<<0);    
    LPC_SYSCON -> SYSAHBCLKCTRL |= (0x1<<11);
    LPC_SYSCON -> SSP0CLKDIV = prescaler;	 
    LPC_IOCON -> SCK_LOC = 0x02;
    LPC_IOCON -> PIO0_6 = 0x02;
    LPC_GPIO0 -> DIR |= (1<<6);

    LPC_IOCON -> PIO0_8 &= ~0x07;	        
    LPC_IOCON -> PIO0_8 |= 0x01;		/* SSP MISO */
    LPC_IOCON -> PIO0_9 &= ~0x07;
    LPC_IOCON -> PIO0_9 |= 0x01;		/* SSP MOSI */
    
    LPC_IOCON -> SCK_LOC = 0x01;
    LPC_IOCON -> PIO2_11 = 0x01;	/* P2.11 is SSP clock*/    
    LPC_GPIO2 -> DIR |= (1<<11);

}


void ssp_init(void){
  uint8_t  Dummy=Dummy;
  
  ssp_io_init(3);
  LPC_SSP0 -> CR0 = (0x07)|(0<<4)|(1<<6)|(1<<7)|(20<<8); 
  LPC_SSP0 -> CPSR = 0x2;  
  LPC_SSP0 -> CR1 = SSPCR1_SSE;
}

void send_command(int command){
  uint8_t Dummy =0;
  Dummy =ssp_communication(command);
}


uint8_t ssp_communication(uint8_t t_buf){
    uint8_t  r_buf = 0;
    xprintf("test\n");

    LPC_GPIO0 -> DATA &= ~(1<<10);
    while ((LPC_SSP0 -> SR & ((1<<1)|(1<<4))) != (1<<1));
    LPC_SSP0 -> DR = t_buf;
    while ( LPC_SSP0 -> SR & (1<<4));

    while ((LPC_SSP0 -> SR & (1<<2)) != (1<<2));
    r_buf = LPC_SSP0 -> DR;    
    LPC_GPIO0 -> DATA |= (1<<10);

    return r_buf;
} 



void ssp_receive(uint8_t *buf){
      while ( (LPC_SSP0 -> SR & (SSPSR_BSY|SSPSR_RNE)) != SSPSR_RNE );
      *buf = LPC_SSP0 -> DR;
}


