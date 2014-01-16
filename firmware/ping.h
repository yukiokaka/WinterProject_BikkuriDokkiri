#ifndef PING_H_
#define PING_H_

int ping(void);
void ping_init(void);
void CT16B1_IRQHandler(void);

extern volatile unsigned char device_num;

#endif
