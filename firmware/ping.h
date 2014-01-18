#ifndef PING_H_
#define PING_H_

#define CMD_PONG 16
#define CMD_PING 32
#define CMD_RESET 24
#define  disable_ping_timer() LPC_TMR16B1 -> TCR = 0
#define  enable_ping_timer() LPC_TMR16B1 -> TCR = 1

extern volatile unsigned char device_num;
extern int ping_enable ;
extern int pong_enable;
extern int send_data_flg;


int ping(void);
int pong(void);
void ping_init(void);
void CT16B1_IRQHandler(void);


#endif
