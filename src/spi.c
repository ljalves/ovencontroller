/* basic SPI communication routines */

#include <avr/io.h>
#include <util/delay.h>
#include "spi.h"

void init_spi (void)
{
	/* DDR register */
	/* 0 = input, 1 = output */

	/* MISO = input */
	DDRB &= ~(1 << DDB6);

	/* MOSI SCK RTC_CS = output */
	DDRB |= (1 << DDB7) | (1 << DDB5) | (1 << DDB1);

	/* T_CS = output */
	DDRD |= (1 << DDD4);


	/* set default levels */
	/* RTC_CS = 1*/
	PORTB |= 1 << PB1;

	/* T_CS = 1 */
	PORTD |= 1 << PD4;

}


void read_spi(chip_cs_t cs, unsigned char len, char *buf)
{
	unsigned char i, j;
	
	if (cs == TEMP_CS)
		PORTD &= ~(1<<PD4);

	/* clock = 0 */
	PORTB &= ~(1<<PB7);

	for (i=0; i<len; i++) {
		buf[i] = 0;

		for (j=0; j<8; j++) {
			/* clock = 1 */
			PORTB |= (1<<PB7);
			_delay_ms(1);

			/* read bit */
			buf[i] <<= 1;
			buf[i] |= ((PINB & (1 << PB6)) == 0 ? 0 : 1);

			/* clock = 0 */
			PORTB &= ~(1<<PB7);
			_delay_ms(1);
		}
	}

	if (cs == TEMP_CS)
		PORTD |= (1<<PD4);
}




