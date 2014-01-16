#include "LPC11xx.h"
#include "display.h"
#include "xprintf.h"

#define _BV(x) (1 << (x))


void matrix_init() {
    LPC_IOCON -> PIO0_7 = 0;
    LPC_IOCON -> PIO1_9 = 0;
    LPC_IOCON -> PIO1_10 = 0;
    LPC_IOCON -> PIO1_11 = 0;
    LPC_IOCON -> PIO2_0 = 0;
    LPC_IOCON -> PIO2_1 = 0;
    LPC_IOCON -> PIO2_2 = 0;
    LPC_IOCON -> PIO2_6 = 0;
    LPC_IOCON -> PIO2_7 = 0;
    LPC_IOCON -> PIO2_8 = 0;
    LPC_IOCON -> PIO2_9 = 0;
    LPC_IOCON -> PIO2_10 = 0;
    LPC_IOCON-> PIO2_11 = 0;

    LPC_GPIO1 -> DIR |= (_BV(10) | _BV(11));
    LPC_GPIO1 -> DATA &= ~(_BV(10) | _BV(11));

    LPC_GPIO2 -> DIR |= (_BV(2) | _BV(9) | _BV(10) | _BV(11));
    LPC_GPIO2 -> DATA &= ~(_BV(9) | _BV(10) | _BV(11));
    LPC_GPIO0 -> DIR |= _BV(7);
}

void display(short array[]) {
    const int width = 16;
    const int height = 16;
    static int row = 0;
    // decode row into 4-bit binary data, and write it
    int oca = row % 2;
    int ocb = (row / 2) % 2;
    int occ = (row / 4) % 2;
    int ocd = (row / 8) % 2;
    if (oca == 0) LPC_GPIO2 -> DATA &= ~(_BV(9));   // OCA = 0
    else LPC_GPIO2 -> DATA |= _BV(9);               // OCA = 1

    if (ocb == 0) LPC_GPIO2 -> DATA &= ~(_BV(10));  // OCB = 0
    else LPC_GPIO2 -> DATA |= _BV(10);              // OCB = 1

    if (occ == 0) LPC_GPIO2 -> DATA &= ~(_BV(11));  // OCC = 0
    else LPC_GPIO2 -> DATA |= _BV(11);              // OCC = 1

    if (ocd == 0) LPC_GPIO0 -> DATA &= ~(_BV(7));    // OCD = 0
    else LPC_GPIO0 -> DATA |= _BV(7);               // OCD = 1


    // serial input parallel output
    LPC_GPIO1 -> DATA &= ~(_BV(11));                // RCK = 0
    int i;
    // order: 7, 6, .., 1, 0, 15, 14, .., 9, 8
    for (i = 0; i < width; i++) {
        char index = (i <= 7) ? 7 - i : 23 - i;

        LPC_GPIO1 -> DATA &= ~(_BV(10));            // SCK = 0
        if (((array[row] >> (15-index)) & 0x01) == 0) 
            LPC_GPIO2 -> DATA |= _BV(2); // SER = 0
        else
            LPC_GPIO2 -> DATA &= ~_BV(2);           // SER = 1
        LPC_GPIO1 -> DATA |= _BV(10);               // SCK = 1
    }
    LPC_GPIO1 -> DATA |= _BV(11);                   // RCK = 1 (positive edge)

    // next row
    if (row == height) row = 0;
    else row++;
}

short DotPicture_1[16] = {
    0b0000000000000000,
    0b0000000010000000,
    0b0000000110000000,
    0b0000001110000000,
    0b0000011110000000,
    0b0000000110000000,
    0b0000000110000000,
    0b0000000110000000,
    0b0000000110000000,
    0b0000000110000000,
    0b0000000110000000,
    0b0000000110000000,
    0b0000011111100000,
    0b0000011111100000,
    0b0000000000000000,
    0b0000000000000000
};

short DotPicture_2[16] = {
    0b0000000000000000,
    0b0000000000000000,
    0b0000001111000000,
    0b0000111111110000,
    0b0001110000111000,
    0b0001100000011000,
    0b0000000000011000,
    0b0000000000110000,
    0b0000000011100000,
    0b0000000110000000,
    0b0000011100000000,
    0b0000110000000000,
    0b0001111111111000,
    0b0001111111111000,
    0b0000000000000000,
    0b0000000000000000
};

short DotPicture_3[16] = {
    0b0000000000000000,
    0b0000000000000000,
    0b0000001111000000,
    0b0000111111110000,
    0b0001110000111000,
    0b0001100000011000,
    0b0000000000011000,
    0b0000000000110000,
    0b0000000011100000,
    0b0000000000110000,
    0b0000000000011000,
    0b0001100000011000,
    0b0001110000111000,
    0b0000111111110000,
    0b0000001111000000,
    0b0000000000000000
};

short DotPicture_4[16] = {
    0b0000000000000000,
    0b0000000001100000,
    0b0000000001100000,
    0b0000000011100000,
    0b0000000111100000,
    0b0000000111100000,
    0b0000001101100000,
    0b0000011001100000,
    0b0000011001100000,
    0b0000110001100000,
    0b0001111111111000,
    0b0001111111111000,
    0b0000000001100000,
    0b0000000001100000,
    0b0000000000000000,
    0b0000000000000000
};
