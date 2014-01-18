#include "LPC11xx.h"
#include "transmit_display_data.h"
#include "ircomm.h"
#include "xprintf.h"
#include "display_timer.h"
#include "timer_controller.h"
#include "pccomm.h"
#include "ping.h"
void send_display_data(void)
{
    int row = 0;
    unsigned char data = 64;
    unsigned long cnt= 1000*1000*1000;
    
    display_data = line_data;

    data = 1;
    ircomm_send(&data);           
    for(row = 0; row < 16; row++) {
        data = line_data[row] & 0x0f;  
        ircomm_send(&data);
        data = (line_data[row] >> 4) & 0x0f;
        ircomm_send(&data);
        data = (line_data[row] >> 8) & 0x0f;
        ircomm_send(&data);
        data = (line_data[row] >> 12) & 0x0f;
        ircomm_send(&data);
    }
    data = 36;
    ircomm_send(&data);

}

char next_ping = 0;
 
void recv_display_data(int(*gpio_func)(void)) 
{
    
    int row = 0;
    char num = 0, data0 = 0, data1 = 0, data2 = 0, data3 = 0, data4 = 0 ;
    static short display_test_buf[16];
    int i;

    num = ircomm_recv(gpio_func);
    for(row = 0; row < 16; row++) {                
        data0 = ircomm_recv(gpio_func);
        data1 = ircomm_recv(gpio_func);
        data2 = ircomm_recv(gpio_func);
        data3 = ircomm_recv(gpio_func); 
        display_test_buf[row] = (data3 << 12 )|  (data2 << 8 )| (data1 << 4 )| data0 ;
    }
    data4 = ircomm_recv(gpio_func);

    if(data4 == 36) {
        for(row = 0; row < 16; row++) {
            line_data[row] = display_test_buf[row];
        }
        next_ping = 1;
        display_data = (short *)line_data;

        
    }
    
}

void reset_display_data_from_ircomm(void)
{
    int row;
    for(row = 0; row < 16; row++) {
        line_data[row] = 0;
    }
    
}
