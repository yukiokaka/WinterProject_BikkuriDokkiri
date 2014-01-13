#include "LPC11xx.h"
#include "ircomm.h"
#include "xprintf.h"

#define _BV(x) (1 << (x))

volatile int send_state = 0;
volatile int recv_state = 0;
volatile int send_data = 124;

volatile int reader_count = 0; 
volatile char reader_count_start_state = 0;
volatile unsigned long second_counter = 0;

static unsigned long micros(void) 
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

void ircomm_send(char *buf)
{
    send_data++;
    while(reader_count_start_state);
    LPC_GPIO1 -> DATA &= ~_BV(3);
    reader_count = 0;
    reader_count_start_state = 1;
    
}

int ircomm_recv(char *buf)
{
     char IRbit[26] ;			// 受信バッファ
     unsigned long t ;
     int ans , i ;
     unsigned long timeout = 0;
     ans = 0 ;
     t   = 0 ;
          // リーダ部のチェックを行う
     if(LPC_GPIO1->MASKED_ACCESS[1 << 0] == 0) {
          t = micros() ;                        
          // 現在の時刻(us)を得る
          while (LPC_GPIO1->MASKED_ACCESS[1 << 0]== 0) ;	
          // 1(ON)になるまで待つ
          t = micros() - t ;	
          // 0(OFF)の部分をはかる
     }
     
     // リーダ部有りなら処理する(4.5ms以上の0にて判断する)
     if (t >= 4500) {
         i = 0 ;
         while(LPC_GPIO1->MASKED_ACCESS[1 << 0] == 1) {
             timeout++;
             if(timeout == 10000)
                 break;
         }; 
         // ここまでがリーダ部(ON部分)読み飛ばす
         // データ部の読み込み
         while (1) {
             timeout = 0;
             while(LPC_GPIO1->MASKED_ACCESS[1 << 0] == 0){
              timeout++;
              if(timeout == 10000)
                  break;              
             }	          
             // OFF部分は読み飛ばす
             t = micros() ;
             timeout = 0;
             while(LPC_GPIO1->MASKED_ACCESS[1 << 0] == 1)  {
                 timeout++;
                 if(timeout == 10000)
                     break;              
             }             
             // 0(FF)になるまで待つ
             t = micros() - t ;					
             // 1(ON)部分の長さをはかる
             if (t >= 1000)  IRbit[i] = (char)0x31 ;	// ON部分が長い
             else            IRbit[i] = (char)0x30 ;	// ON部分が短い
             i++ ;
             if (i == 8) break ;				// 1byte読込んだら終了
          }
         return(((IRbit[7] == 0x31) << 7) | ((IRbit[6]  == 0x31) << 6) | ((IRbit[5] == 0x31) << 5) | ((IRbit[4]  == 0x31) << 4) | ((IRbit[3] == 0x31) << 3) | ((IRbit[2] == 0x31) << 2) | ((IRbit[1]  == 0x31)<< 1)| ((IRbit[0] == 0x31) << 0)) ;
     }
     return 0;
}


void CT32B0_IRQHandler(void)
{
    static char data_bit_num = 0;
    static char RZ_state = 0, wait_state = 0, high_sig = 0;
    second_counter += 26;
    reader_count++;
    if(reader_count_start_state == 1) {
            if(RZ_state % 2) {
                LPC_GPIO1 -> DATA &= ~_BV(3);
            }
            else {
                LPC_GPIO1 -> DATA |= _BV(3);
            }
            RZ_state++;
            if(reader_count == 384) {
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
