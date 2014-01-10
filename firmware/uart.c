#include "uart.h"

volatile uint32_t UARTStatus=0;
volatile uint8_t  UARTTxEmpty = 1;
volatile uint8_t  UARTBuffer[BUFSIZE];
volatile uint32_t UARTCount = 0;

void uart_init(uint32_t baudrate){
  uint32_t Fdiv;
  uint32_t regVal;

  UARTTxEmpty = 1;
  UARTCount = 0;
  LPC_IOCON -> PIO1_6 &= ~0x07;    
  LPC_IOCON -> PIO1_6 |= 0x01;     /* UART RXD */
  LPC_IOCON -> PIO1_7 &= ~0x07;	
  LPC_IOCON -> PIO1_7 |= 0x01;     /* UART TXD */


  LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 12);
  LPC_SYSCON->UARTCLKDIV = 1;   
  regVal = LPC_SYSCON -> UARTCLKDIV;

  LPC_UART -> ACR = 0;

  LPC_UART -> LCR = 0x80;             /* 8 bits, no Parity, 1 Stop bit */
  LPC_UART -> FDR = (1<<4);


  Fdiv = 36000000 / (regVal * 16 * baudrate) ;/*baud rate */
  LPC_UART -> DLM = Fdiv / 256;
  LPC_UART -> DLL = Fdiv % 256;
  LPC_UART -> LCR &= ~0x80;	   
 

  LPC_UART -> FCR = 0x07;		/* Enable and reset TX and RX FIFO. */
  LPC_UART -> LCR |= 0x03;	 

  
  return;
}


void uart_putc(const char c) {
    if (c == '\n') 
        uart_putc ('\r');
    while(!(LPC_UART -> LSR & LSR_THRE));
    LPC_UART -> THR = c;
}

int uart_puts(const char *s) {
    int n;
    for (n = 0; *s; s++, n++) {
        uart_putc(*s);
    }
    return n;
}


int uart_getc(void) {
    while (!(LPC_UART -> LSR & LSR_RDR));
    return LPC_UART -> RBR;
}


char *uart_gets(char * const buf, const int size) {
    int ch;
    int n = 0;

    while (n < size - 1) {
        ch = uart_getc();
        if (ch == '\r') {
            uart_puts("\r\n");
            break;
        }
        uart_putc(ch);
        buf[n++] = ch;
    }
    buf[n] = 0;
    return buf;
} 
