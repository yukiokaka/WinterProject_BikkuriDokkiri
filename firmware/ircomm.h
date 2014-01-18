#ifndef _IRCOMM_H_
#define _IRCOMM_H_

#define  disable_ircomm_timer() LPC_TMR32B0 -> TCR = 0
#define  enable_ircomm_timer() LPC_TMR32B0 -> TCR = 1

int IR0(void);
int IR1(void);
int IR2(void);
int IR3(void);
void ircomm_init(void);
int ircomm_send(unsigned char *buf);
int ircomm_recv(int (*gpio_func)(void));

void CT32B0_IRQHandler(void);
#endif
