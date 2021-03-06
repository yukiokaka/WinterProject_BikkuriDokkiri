#ifndef __SSP_H__
#define __SSP_H__


#define SSP_SLAVE		0		/* 1 is SLAVE mode, 0 is master mode */
#define TX_RX_ONLY		0		/* 1 is TX or RX only depending on SSP_SLAVE
								flag, 0 is either loopback mode or communicate
								with a serial EEPROM. */


#define USE_CS			0
#define SCK0_11                 0
/* SPI read and write buffer size */
#define SSP_BUFSIZE		16
#define FIFOSIZE		8

#define DELAY_COUNT		10
#define MAX_TIMEOUT		0xFF

/* Port0.2 is the SSP select pin */
#define SSP0_SEL        (0x1<<2)
	
/* SSP Status register */
#define SSPSR_TFE       (0x1<<0)
#define SSPSR_TNF       (0x1<<1) 
#define SSPSR_RNE       (0x1<<2)
#define SSPSR_RFF       (0x1<<3) 
#define SSPSR_BSY       (0x1<<4)

/* SSP CR0 register */
#define SSPCR0_DSS      (0x1<<0)
#define SSPCR0_FRF      (0x1<<4)
#define SSPCR0_SPO      (0x1<<6)
#define SSPCR0_SPH      (0x1<<7)
#define SSPCR0_SCR      (0x1<<8)

/* SSP CR1 register */
#define SSPCR1_LBM      (0x1<<0)
#define SSPCR1_SSE      (0x1<<1)
#define SSPCR1_MS       (0x1<<2)
#define SSPCR1_SOD      (0x1<<3)

/* SSP Interrupt Mask Set/Clear register */
#define SSPIMSC_RORIM   (0x1<<0)
#define SSPIMSC_RTIM    (0x1<<1)
#define SSPIMSC_RXIM    (0x1<<2)
#define SSPIMSC_TXIM    (0x1<<3)

/* SSP0 Interrupt Status register */
#define SSPRIS_RORRIS   (0x1<<0)
#define SSPRIS_RTRIS    (0x1<<1)
#define SSPRIS_RXRIS    (0x1<<2)
#define SSPRIS_TXRIS    (0x1<<3)

/* SSP0 Masked Interrupt register */
#define SSPMIS_RORMIS   (0x1<<0)
#define SSPMIS_RTMIS    (0x1<<1)
#define SSPMIS_RXMIS    (0x1<<2)
#define SSPMIS_TXMIS    (0x1<<3)

/* SSP0 Interrupt clear register */
#define SSPICR_RORIC    (0x1<<0)
#define SSPICR_RTIC     (0x1<<1)


/* RDSR status bit definition */
#define RDSR_RDY	0x01
#define RDSR_WEN	0x02

#define	CS_L() LPC_GPIO0 -> DATA &= ~(1<<10)/* Set MMC CS "high" */
#define CS_H() LPC_GPIO0 -> DATA |= (1<<10);/* Set MMC CS "low" */
#define	CK_L() LPC_GPIO0 -> DATA &= ~(1<<6)/* Set MMC CS "high" */
#define CK_H() LPC_GPIO0 -> DATA |= (1<<6);/* Set MMC CS "low" */
#define	DI_L() LPC_GPIO0 -> DATA &= ~(1<<9)/* Set MMC CS "high" */
#define DI_H() LPC_GPIO0 -> DATA |= (1<<9);/* Set MMC CS "low" */
#define DO 	 LPC_GPIO0 -> DATA |= (1<<9);/* Get MMC DO value (high:true, low:false) */



void ssp_init(void);
void ssp_send(int command);
uint8_t  ssp_communication(uint8_t t_buf);
void ssp_receive(uint8_t *buf);

#endif

