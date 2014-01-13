#ifndef _IRCOMM_H_
#define _IRCOMM_H_

void ircomm_init(void);
void ircomm_send(char *buf);
void ircomm_recv(char *buf);

void CT32B0_IRQHandler(void);
#endif
