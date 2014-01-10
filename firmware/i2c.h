#ifndef I2C_H_
#define I2C_H_

#define CLR_ACK 0
#define SET_ACK 1
#define WRITE 0
#define READ  1


void i2c_start(void);
void i2c_stop(void);
char i2c_send_slave_address(char Address, char rw);
void i2c_recv_byte(int *data,char AA);
void i2c_send_byte(int *data,char AA);
void i2c_init(void);
#endif
