#include "LPC11xx.h"
#include "transmit_display_data.h"
#include "ircomm.h"
#include "xprintf.h"
#include "display_timer.h"
#include "timer_controller.h"
#include "pccomm.h"

void send_display_data(void)
{
    int row = 0;
    unsigned char data = 64;
    unsigned long cnt= 1000*1000*1000;
    
    display_data = line_data;
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
}


void recv_display_data(void) 
{
    
    int row = 0;
    char data0 = 0, data1 = 0, data2 = 0, data3 = 0 ;
    static short display_test_buf[16];
    int i;
    
    for(row = 0; row < 16; row++) {
        data0 = ircomm_recv();
        for(i=0;i<50000;i++);
        data1 = ircomm_recv();
        for(i=0;i<50000;i++);
        data2 = ircomm_recv();
        for(i=0;i<50000;i++);
        data3 = ircomm_recv();

        display_test_buf[row] = (data3 << 12 )|  (data2 << 8 )| (data1 << 4 )| data0 ;
    }
    
    
    for(row = 0; row < 16; row++) {
        line_data[row] |= display_test_buf[row];
    }
    display_data = (short *)line_data;
    
}

void reset_display_data_from_ircomm(void)
{
    int row;
    for(row = 0; row < 16; row++) {
        line_data[row] = 0;
    }
    
}
