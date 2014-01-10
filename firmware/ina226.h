#ifndef INA226_H_
#define INA226_H_

#define INA226_ADDRESS  64


#define REG_CONFIGURATION 0x00
#define REG_SHUNT_VOLTAGE 0x01
#define REG_BUS_VOLTAGE   0x02
#define REG_POWER         0x03
#define REG_CURRENT       0x04
#define REG_CALIBRATION   0x05
#define REG_MASK_ENABLE   0x06
#define REG_ALERT_LIMIT   0x07


void ina226_init(void);
void ina226_select_register(int reg);
void ina226_send_register(int *data, int reg);
void ina226_recv_register(int *data, int reg);

#endif
