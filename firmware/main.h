#ifndef _MAIN_H_
#define _MAIN_H_
#define TIMEOUT  500

#define SLAVE_MODE 0
#define HOST_MODE 1

extern int Mode;

int main (void);
void SysTick_Handler (void);

#endif
