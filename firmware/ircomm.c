#include "LPC11xx.h"
#include "ircomm.h"
#include "xprintf.h"

#define _BV(x) (1 << (x))

volatile int send_state = 0;
volatile int recv_state = 0;
volatile int send_data = 0x00;

volatile int reader_count = 0; 
volatile char reader_count_start_state = 0;

void ircomm_init(void)
{
	LPC_SYSCON->SYSAHBCLKCTRL |=  (1 << 9);
    LPC_TMR32B0 -> PR = (3*2)-1;			
    LPC_TMR32B0 -> MR3 = 26-1;			
    LPC_TMR32B0 -> MCR &= ~(7 << 9);	
    LPC_TMR32B0 -> MCR |=  (3 << 9);	
    NVIC -> ISER[0] |= (1<<18);
    LPC_TMR32B0->TCR = 1;
}

void ircomm_send(char *buf)
{
    while(reader_count_start_state);
    LPC_GPIO1 -> DATA &= ~_BV(3);
    reader_count = 0;
    reader_count_start_state = 1;
    
}

void ircomm_recv(char *buf)
{
    *buf = 0; 
    
}


void CT32B0_IRQHandler(void)
{
    static char data_bit_num = 0;
    static char RZ_state = 0, wait_state = 0, high_sig = 0;

    reader_count++;
    if(reader_count == 384) {
        if(reader_count_start_state == 1) {            
            LPC_GPIO1 -> DATA |= _BV(3);
            reader_count = 0;
            reader_count_start_state = 2;            
        }  
    }
    else if(reader_count == 38) {
        if(reader_count_start_state == 2) {            
            reader_count = 0;
            data_bit_num = 0;
            RZ_state = 0;
            high_sig = 1;
            reader_count_start_state = 3;            
        } 
    }   
    if(reader_count_start_state == 3) {
        if(high_sig) {
            if(RZ_state % 2) {
                LPC_GPIO1 -> DATA &= ~_BV(3);
            }
            else {
                LPC_GPIO1 -> DATA |= _BV(3);
            }
            RZ_state++;
        }
        if(reader_count == 21)  {
            LPC_GPIO1 -> DATA |= _BV(3);
            reader_count = 0;
            if(high_sig) {
                high_sig = 0;
                if(send_data&(1<<data_bit_num))
                    wait_state = 2;
                else 
                    wait_state = 1;
            }           
            else
                wait_state--;

            if(!wait_state) {
                data_bit_num++;
                if(data_bit_num == 8)
                    reader_count_start_state = 0;
                else
                    high_sig = 1;
            }
        
        }
    }
	LPC_TMR32B0->IR =0x8;
    
}
