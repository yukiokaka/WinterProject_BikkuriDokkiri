#ifndef _IRCOMM_H_
#define _IRCOMM_H_

void ircomm_init(void);
int ircomm_send(char *buf);
int ircomm_recv(char *buf);

void CT32B0_IRQHandler(void);
#endif
