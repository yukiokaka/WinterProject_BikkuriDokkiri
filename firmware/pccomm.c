#include "LPC11xx.h"
#include "pccomm.h"
#include "uart.h"
#include "ping.h"
#include "display_timer.h"
#include "timer_controller.h"
#include "xprintf.h"
#include "main.h"
#include "transmit_display_data.h"

#define RECV_PC_SEND_SIGNAL 0
#define SEND_PC_DATA_SIGNAL 1
#define SEND_PC_FIN_SIGNAL  2

volatile short line_data[16] = {
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000
};

inline void wait_p(void)
{
    unsigned char recv_data = 0;
    while(1) {
        while((((LPC_UART->LSR & LSR_RDR) != LSR_RDR)));//wait
        recv_data = LPC_UART -> RBR;
        if(recv_data == 'P')
            break;
    }
    
}

int pc_state_machine(void) 
{
    static int mode = RECV_PC_SEND_SIGNAL;
    
    unsigned char num = 0;
    unsigned char line_num = 0;
    
    unsigned char recv_data;
    unsigned char line_data0 = 0;
    unsigned char line_data1 = 0;
    int i;
    volatile short line_data_buf[16];
    
    if(mode == RECV_PC_SEND_SIGNAL) {
        if(((LPC_UART->LSR & LSR_RDR) == LSR_RDR)) {
            recv_data = LPC_UART -> RBR;
            
            if(recv_data == 'S') {
                xprintf("R");
                /*--UART 通信モードに突入---*/
                /*--Timerをすべて停止-------*/
                Mode = HOST_MODE;
                disable_timer();
                line_num = 0;
                mode = SEND_PC_DATA_SIGNAL;
            }
        } 
    }
    else if(mode == SEND_PC_DATA_SIGNAL) {
        while(num < 1) {

            wait_p();
            uart_putc('0');                        
            uart_putc(num + 0x30);

            wait_p();
            uart_putc(line_num / 10 + 0x30);
            uart_putc(line_num % 10 + 0x30);
            uart_putc('\0');
            
            wait_p();

            while((((LPC_UART->LSR & LSR_RDR) != LSR_RDR)));
            line_data0 = LPC_UART -> RBR; //データ1をバッファリング
            while((((LPC_UART->LSR & LSR_RDR) != LSR_RDR))); //データ1を受信

            line_data0 = LPC_UART -> RBR; //データ1をバッファリング
            uart_putc('C');
            uart_putc('\0');

            wait_p();
            while((((LPC_UART->LSR & LSR_RDR) != LSR_RDR)));
            line_data1 = LPC_UART -> RBR; //データ2をバッファリング
            
            while((((LPC_UART->LSR & LSR_RDR) != LSR_RDR)));

            line_data1 = LPC_UART -> RBR;

            line_data_buf[15-line_num] = (line_data1 << 8) | line_data0;
  
            line_num++;

            if(line_num == 16) {
                num++;
                line_num = 0;
            }
            
        }
        mode = SEND_PC_FIN_SIGNAL;
    }
    if(mode == SEND_PC_FIN_SIGNAL) {
        uart_putc('F');
        for(i = 0; i < 16; i++) { 
            line_data[i] = line_data_buf[i];
        }
        display_data = (short *)line_data;
        
        
        mode = RECV_PC_SEND_SIGNAL;
    }

    /*---UART通信モードを離脱---*/
    /*---Timerをすべて再起動---*/
    
    enable_timer();
    
    return 0;
}
