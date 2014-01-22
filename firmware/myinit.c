#include "LPC11xx.h"		/* LPC11xx register definitions */
#include "ircomm.h"
#include "display_timer.h"
#include "ping.h"
#include "system_LPC11xx.h"
#include "main.h"


#define _BV(x) (1 << (x))
/* Section boundaries defined in linker script */
extern long _sidata[], _sdata[], _edata[], _sbss[], _ebss[], _endof_sram[];


/* copied from http://cortexm.blog134.fc2.com/ */
typedef struct{
    __IO uint32_t FLASHCFG; /*!< Offset: 0x000 Flash configuration Register (R/W) */
}LPC_FLASH_TypeDef;

#define LPC_FLASHC_BASE (LPC_APB0_BASE + 0x3C010)
#define LPC_FLASH ((LPC_FLASH_TypeDef *) LPC_FLASHC_BASE )


/*--------------------------------------------------------------------/
  / Program Core                                                        /
  /--------------------------------------------------------------------*/

void MySystemInit(void)
{
    long *s, *d;
    int i;

    /* Configure BOD control (Reset on Vcc dips below 2.7V) */
    LPC_SYSCON->BODCTRL = 0x13;

    /* Configure system clock generator (36MHz system clock with Crystal) */
    LPC_SYSCON->MAINCLKSEL = 0;							/* Select IRC as main clock */
    /* In order for the update to take effect, first write
       a zero to the MAINCLKUEN register and then write a one to MAINCLKUEN. */
    LPC_SYSCON->MAINCLKUEN = 0; LPC_SYSCON->MAINCLKUEN = 1;
    LPC_SYSCON->PDRUNCFG &= ~_BV(5);          /* Power-up System Osc      */
    for (i = 0; i < 200; i++) __NOP();
    LPC_FLASH->FLASHCFG = (LPC_FLASH->FLASHCFG & 0xFFFFFFFC) | 0;	/* Set wait state for flash memory (0WS) */
    LPC_SYSCON->SYSPLLCLKSEL = 1;	/* Select System Oscillator for PLL-in */

    //PLLを使用する場合///////////
    LPC_SYSCON->SYSPLLCTRL   = (2<<5)|(2<<0);
    LPC_SYSCON->SYSPLLCLKUEN =1;
    LPC_SYSCON->SYSPLLCLKUEN =0;
    LPC_SYSCON->SYSPLLCLKUEN =1;
    while (!(LPC_SYSCON->SYSPLLCLKUEN & 0x01));       /* Wait Until Updated       */
    LPC_SYSCON->PDRUNCFG     &= ~(1 << 7);          /* Power-up SYSPLL          */
    while (!(LPC_SYSCON->SYSPLLSTAT & 0x01));	      /* Wait Until PLL Locked    */


    LPC_SYSCON->SYSAHBCLKDIV = 3;	/* Set system clock divisor (1) */
    LPC_SYSCON->MAINCLKSEL = 3;/* Select PLL-in as main clock */
    LPC_SYSCON->MAINCLKUEN    = 0x01;               /* Update MCLK Clock Source */
    LPC_SYSCON->MAINCLKUEN    = 0x00;               /* Toggle Update Register   */
    LPC_SYSCON->MAINCLKUEN    = 0x01;
    while (!(LPC_SYSCON->MAINCLKUEN & 0x01));       /* Wait Until Updated       */

    /* Enable clock for only SYS, ROM, RAM, FLASH, GPIO and IOCON */
    LPC_SYSCON->SYSAHBCLKCTRL = _BV(0) | _BV(1) | _BV(2) | _BV(3) | _BV(4) | _BV(6) | _BV(16);

    /* Initialize .data/.bss section and static objects get ready to use after this process */
    for (s = _sidata, d = _sdata; d < _edata; *d++ = *s++) ;
    for (d = _sbss; d < _ebss; *d++ = 0) ;

    SystemCoreClock = 12000000;

    /*------- END OF SYSTEM INITIALIZATION -------*/
}

/*--------------------------------------------------------------------/
  / Exception Vector Table                                              /
  /--------------------------------------------------------------------*/

void trap (void)
{
    for (;;) ;	/* Trap spurious interrupt */
}

void* const vector[] __attribute__ ((section(".VECTOR"))) =	/* Vector table to be allocated to address 0 */
{
    _endof_sram,	/* Reset value of MSP */
    main,			/* Reset entry */
    trap,//NMI_Handler,
    trap,//HardFault_Hander,
    0, 0, 0, 0, 0, 0, 0,//<Reserved>
    trap,//SVC_Handler,
    0, 0,//<Reserved>
    trap,//PendSV_Handler,
    SysTick_Handler,
    trap,//PIO0_0_IRQHandler,
    trap,//PIO0_1_IRQHandler,
    trap,//PIO0_2_IRQHandler,
    trap,//PIO0_3_IRQHandler,
    trap,//PIO0_4_IRQHandler,
    trap,//PIO0_5_IRQHandler,
    trap,//PIO0_6_IRQHandler,
    trap,//PIO0_7_IRQHandler,
    trap,//PIO0_8_IRQHandler,
    trap,//PIO0_9_IRQHandler,
    trap,//PIO0_10_IRQHandler,
    trap,//PIO0_11_IRQHandler,
    trap,//PIO1_0_IRQHandler,
    trap,//C_CAN_IRQHandler,
    trap,//SPI1_IRQHandler,
    trap,//I2C_IRQHandler,
    trap,//CT16B0_IRQHandler,
    CT16B1_IRQHandler,
    CT32B0_IRQHandler,//CT32B0_IRQHandler,
    CT32B1_IRQHandler,//CT32B1_IRQHandler,
    trap,//SPI0_IRQHandler,
    trap,//UART_IRQHandler,
    0, 0,//<Reserved>
    trap,//ADC_IRQHandler,
    trap,//WDT_IRQHandler,
    trap,//BOD_IRQHandler,
    0,//<Reserved>
    trap,//PIO_3_IRQHandler,
    trap,//PIO_2_IRQHandler,
    trap,//PIO_1_IRQHandler,
    trap //PIO_0_IRQHandler
};


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

