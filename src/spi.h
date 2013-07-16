#ifndef _SPI_H_
#define _SPI_H_



typedef enum {
	TEMP_CS,
	RTC_CS,
} chip_cs_t;





void init_spi(void);
void read_spi(chip_cs_t cs, unsigned char len, unsigned char *buf);


#endif
