#ifndef DEF_TERMINAL
#define DEF_TERMINAL

extern volatile UINT Timer;	/* 1kHz increment timer */

void terminal_init(void);
void terminal(void);
#endif
