#include "LPC11xx.h"
#include "ircomm.h"
#include "xprintf.h"

#define _BV(x) (1 << (x))

volatile int send_state = 0;
volatile int recv_state = 0;
volatile unsigned char send_data_ir = 124;
volatile int send_data = 0;
volatile int reader_count = 0; 
volatile char reader_count_start_state = 0;
volatile unsigned long second_counter = 0;
int IRLED_NUM = 0;

void (*IRLED_ON[])(void) ={IRLED0_ON, IRLED1_ON, IRLED2_ON, IRLED3_ON, IRLEDALL_ON };
void (*IRLED_OFF[])(void) ={IRLED0_OFF, IRLED1_OFF, IRLED2_OFF, IRLED3_OFF, IRLEDALL_OFF };

void IRLEDALL_ON(void)
{
    LPC_GPIO1 -> DATA |= _BV(3);    
    LPC_GPIO1 -> DATA |= _BV(4);    
    LPC_GPIO1 -> DATA |= _BV(5);    
    LPC_GPIO1 -> DATA |= _BV(8);    

}

void IRLEDALL_OFF(void)
{
    LPC_GPIO1 -> DATA &= ~_BV(3);    
    LPC_GPIO1 -> DATA &= ~_BV(4);    
    LPC_GPIO1 -> DATA &= ~_BV(5);    
    LPC_GPIO1 -> DATA &= ~_BV(8);    

}


void IRLED0_ON(void) 
{
    LPC_GPIO1 -> DATA |= _BV(3);    
}

void IRLED0_OFF(void) 
{
    LPC_GPIO1 -> DATA &=~_BV(3);    
}

void IRLED1_ON(void) 
{
    LPC_GPIO1 -> DATA |= _BV(4);    
}

void IRLED1_OFF(void) 
{
    LPC_GPIO1 -> DATA &=~_BV(4);    
}

void IRLED2_ON(void) 
{
    LPC_GPIO1 -> DATA |= _BV(5);    
}

void IRLED2_OFF(void)
{
    LPC_GPIO1 -> DATA &=~_BV(5);    
}
void IRLED3_ON(void) 
{
    LPC_GPIO1 -> DATA |= _BV(8);    
}

void IRLED3_OFF(void) 
{
    LPC_GPIO1 -> DATA &=~_BV(8);    
}


int IR0(void) {
    return LPC_GPIO0->MASKED_ACCESS[1 << 11];
}

int IR1(void) {
    return LPC_GPIO1->MASKED_ACCESS[1 << 0];
}
int IR2(void) {
    return LPC_GPIO1->MASKED_ACCESS[1 << 1];
}
int IR3(void) {
    return LPC_GPIO1->MASKED_ACCESS[1 << 2];
}


static inline unsigned long micros(void) 
{
    return second_counter;
}
    
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

int ircomm_send(unsigned char *buf)
{
    unsigned char send_data_ir_bar;
    int time = 0;
    while(reader_count_start_state) {
        time++;
        if(time == 10000) return 0;
    };    
    send_data_ir = *buf;
    send_data_ir_bar = ~send_data_ir;
    send_data = (send_data_ir_bar << 8 | send_data_ir);
    //    LPC_GPIO1 -> DATA &= ~_BV(3);
    reader_count = 0;
    reader_count_start_state = 1;
    
    return 0;
    
}



int ircomm_recv(int (*gpio_func)(void))
{
     char IRbit[26] ;			// 受信バッファ
     unsigned long t ;
     int ans , i ;
     unsigned long timeout = 0, timeout2 = 0;
     unsigned char data = 0;
     unsigned char data_bar = 0;
     ans = 0 ;
     t   = 0 ;
     // リーダ部のチェックを行う               
     if(gpio_func() == 0) {
         t = micros() ;                        
         // 現在の時刻(us)を得る
         while (gpio_func()== 0) ;	
         // 1(ON)になるまで待つ
         t = micros() - t ;	
         // 0(OFF)の部分をはかる
     }
     // リーダ部有りなら処理する(4.5ms以上の0にて判断する)
     if (t >= 4500) {
         i = 0 ;
         while(gpio_func());
         // ここまでがリーダ部(ON部分)読み飛ばす
         // データ部の読み込み
         while (1) {
             while(gpio_func() == 0);
             // OFF部分は読み飛ばす
             t = micros() ;
             while(gpio_func() );
             // 0(FF)になるまで待つ
             t = micros() - t ;					
             // 1(ON)部分の長さをはかる
             if (t >= 1000)  IRbit[i] = (char)0x31 ;	// ON部分が長い
             else            IRbit[i] = (char)0x30 ;	// ON部分が短い
             i++ ;
             if (i == 16) break ;				// 1byte読込んだら終了
         }

         data = ((IRbit[7] == 0x31) << 7) | ((IRbit[6]  == 0x31) << 6) | ((IRbit[5] == 0x31) << 5) | ((IRbit[4]  == 0x31) << 4) | ((IRbit[3] == 0x31) << 3) | ((IRbit[2] == 0x31) << 2) | ((IRbit[1]  == 0x31)<< 1)| ((IRbit[0] == 0x31) << 0);
         data_bar = ((IRbit[15] == 0x31) << 7) | ((IRbit[14]  == 0x31) << 6) | ((IRbit[13] == 0x31) << 5) | ((IRbit[12]  == 0x31) << 4) | ((IRbit[11] == 0x31) << 3) | ((IRbit[10] == 0x31) << 2) | ((IRbit[9]  == 0x31)<< 1)| ((IRbit[8] == 0x31) << 0);

         if (data > 128 ) data = data - 128;

         if(data == 255-data_bar)
             return data;
         else
             return 0;
     }
     return 0;
}


void CT32B0_IRQHandler(void)
{
    static char data_bit_num = 0;
    static char RZ_state = 0, wait_state = 0, high_sig = 0;
    second_counter += 26;
    
    //    if(reader_count_start_state)
        reader_count++;
    
    if(second_counter == 1000*1000*1000*1000)
        second_counter = 0;

    if(reader_count_start_state == 1) {
            if(RZ_state % 2) {
                IRLED_OFF[IRLED_NUM]();
            }
            else {
                IRLED_ON[IRLED_NUM]();
            }
            RZ_state++;
            if(reader_count == 384) {
                reader_count = 0;
                IRLED_OFF[IRLED_NUM]();
                reader_count_start_state = 2;
            }            
    }  
    
    else if(reader_count == 384) {
        if(reader_count_start_state == 2) {            
            IRLED_OFF[IRLED_NUM]();
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
                IRLED_OFF[IRLED_NUM]();
            }
            else {
                IRLED_ON[IRLED_NUM]();
            }
            RZ_state++;
        }
        if(reader_count == 21)  {
            IRLED_ON[IRLED_NUM]();
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
                if(data_bit_num == 16) {
                    LPC_GPIO1 -> DATA &=~_BV(3);

                    reader_count_start_state = 0;
                    reader_count = 0;
                }
                else
                    high_sig = 1;
            }
        
        }
    }
    
	LPC_TMR32B0->IR =0x8;
    
}
