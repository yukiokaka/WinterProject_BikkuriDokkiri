#include "LPC11xx.h"
#include "i2c.h"

void i2c_init(void)
{
	LPC_IOCON -> PIO0_4 = 0x1;     //SCL
	LPC_IOCON -> PIO0_5 = 0x1;     //SDA

	LPC_SYSCON -> PRESETCTRL |= 1 << 1;      //I2C reset
	LPC_SYSCON -> SYSAHBCLKCTRL |= 1 << 5;   //I2C clock enabled

	LPC_I2C -> CONSET = 1 << 6;   
	LPC_I2C -> SCLH = 15;         
	LPC_I2C -> SCLL = 15;
	LPC_I2C -> CONCLR = 1 << 3 | 1 << 5;   
}


void i2c_start(void)
{
    LPC_I2C -> CONSET =  ( 1<< 5 | 1<<6);    
    while(!(LPC_I2C -> CONSET & 1 << 3));
    LPC_I2C -> CONCLR = 1 << 5;     

}

void i2c_stop(void)
{
    /* SendSTOP bit */
    LPC_I2C -> CONSET = 1 << 4;
    LPC_I2C -> CONCLR = 1 << 3; 

}


char i2c_send_slave_address(char Address, char rw)
{
    char status = 0;
    /* Send Slave Address */
    LPC_I2C -> DAT =  Address << 1 | rw;
    LPC_I2C -> CONSET = 1 << 2;    
    LPC_I2C -> CONCLR = 1 << 3; 
    while(!(LPC_I2C -> CONSET & 1 << 3));
    
    status = LPC_I2C -> STAT;
    return status;

}

void i2c_recv_byte(int *data,char AA)
{
    if(AA == SET_ACK)
        LPC_I2C -> CONSET = 1 << 2; 
    else
        LPC_I2C -> CONCLR = 1 << 2; 
    LPC_I2C -> CONCLR = 1 << 3; 
    while(!(LPC_I2C -> CONSET & 1 << 3));
    *data = LPC_I2C ->DAT;
  
}

void i2c_send_byte(int *data,char AA)
{
    if(AA == 1)
        LPC_I2C -> CONSET = 1 << 2; 
    else
        LPC_I2C -> CONCLR = 1 << 2; 
    LPC_I2C ->DAT = *data;
    LPC_I2C -> CONCLR = 1 << 3; 
    while(!(LPC_I2C -> CONSET & 1 << 3));

}
