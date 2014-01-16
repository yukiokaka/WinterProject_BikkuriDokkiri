#ifndef _IRCOMM_H_
#define _IRCOMM_H_

#define  disable_ircomm_timer() LPC_TMR32B0 -> TCR = 0
#define  enable_ircomm_timer() LPC_TMR32B0 -> TCR = 1


void ircomm_init(void);
int ircomm_send(unsigned char *buf);
int ircomm_recv(void);

void CT32B0_IRQHandler(void);
#endif
