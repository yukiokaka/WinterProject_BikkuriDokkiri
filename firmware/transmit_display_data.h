#ifndef TRANS_DISPLAY_DATA_H_
#define TRANS_DISPLAY_DATA_H_
extern char next_ping;
uint8_t get_display(void);
int set_display(short *display_data, int size, int row);
void send_display_data(void);
void recv_display_data(int(*gpio_func)(void));
#endif
