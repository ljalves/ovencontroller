/* basic SPI communication routines */

#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "spi.h"

void init_spi (void)
{
	/* DDR register */
	/* Setup pins as input and output */
	/* 0 = input, 1 = output */

	/* MISO = input */
	DDRB &= ~(1 << DDB6);

	/* MOSI SCK RTC_CS = output */
	DDRB |= (1 << DDB7) | (1 << DDB5) | (1 << DDB1);

	/* T_CS = output */
	DDRD |= (1 << DDD4);


	/* Set default pin levels */
	/* RTC_CS = 1*/
	PORTB |= 1 << PB1;

	/* T_CS = 1 */
	PORTD |= 1 << PD4;
}


void read_spi(chip_cs_t cs, unsigned char len, unsigned char *buf)
{
	unsigned char i, j;

	/* assert chip select pin */
	switch (cs) {
	case TEMP_CS:
		PORTD &= ~(1<<PD4);
		break;
	case RTC_CS:
		PORTB &= ~(1<<PB1);
		break;
	}

	/* clock = 0 */
	PORTB &= ~(1<<PB7);

	for (i=0; i<len; i++) {
		buf[i] = 0;

		for (j=0; j<8; j++) {
			/* clock = 1 */
			PORTB |= (1<<PB7);
			_delay_us(1);

			/* read bit */
			buf[i] <<= 1;
			buf[i] |= ((PINB & (1 << PB6)) == 0 ? 0 : 1);

			/* clock = 0 */
			PORTB &= ~(1<<PB7);
			_delay_us(1);
		}
	}

	/* de-assert chip select */
	switch (cs) {
	case TEMP_CS:
		PORTD |= (1<<PD4);
		break;
	case RTC_CS:
		PORTB |= (1<<PB1);
		break;
	}
}




