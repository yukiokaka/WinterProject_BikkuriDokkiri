#include "LPC11xx.h"
#include "ina226.h"
#include "i2c.h"

static int readable_register = 0;

void ina226_init(void)
{
    int configuartion_data = 6;

    i2c_init();
    ina226_send_register(&configuartion_data, REG_CONFIGURATION);
    ina226_select_register(REG_BUS_VOLTAGE);
}


void ina226_select_register(int reg)
{
    char status = 0;
    i2c_start();
   
    status = i2c_send_slave_address(INA226_ADDRESS, WRITE);
    i2c_send_byte(&reg, SET_ACK);
    i2c_stop();
    
    if(status == 0x40)
        readable_register = reg;
}


void ina226_recv_register(int *data, int reg)
{
    int data_h, data_l;
    char status = 0;

    if(readable_register != reg) {
        ina226_init();
        ina226_select_register(reg);
    }

    i2c_start();
    status = i2c_send_slave_address(INA226_ADDRESS, READ);
    i2c_recv_byte(&data_h, SET_ACK);
    i2c_recv_byte(&data_l, CLR_ACK);
    i2c_stop();
    
    if(status == 0x40)
        *data = (data_h << 8) | data_l;
    else
        *data = 0;
}


void ina226_send_register(int *data, int reg)
{
    int data_l = *data & 0xff, data_h = (*data >> 8) &0xff;
    char status = 0;

    if(readable_register != reg) {
        ina226_init();
        ina226_select_register(reg);
    }

    i2c_start();
    status = i2c_send_slave_address(INA226_ADDRESS, WRITE);
    i2c_send_byte(&data_h, SET_ACK);
    i2c_send_byte(&data_l, SET_ACK);
    i2c_stop();
}
