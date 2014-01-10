
#ifndef _UART_H_ 
#define _UART_H_


#include "LPC11xx.h"

#define RS485_ENABLED		0
#define TX_INTERRUPT		0		
#define MODEM_TEST			0

#define IER_RBR		0x01
#define IER_THRE	0x02
#define IER_RLS		0x04

#define IIR_PEND	0x01
#define IIR_RLS		0x03
#define IIR_RDA		0x02
#define IIR_CTI		0x06
#define IIR_THRE	0x01

#define LSR_RDR		0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80

#define BUFSIZE		0x40
#define ACR_START       0x01



void uart_init(uint32_t Baudrate);
void uart_putc(const char c);
int uart_puts(const char *s);
int uart_getc(void);
char *uart_gets(char * const buf, const int size);

#endif /* end _UART_H_ */
