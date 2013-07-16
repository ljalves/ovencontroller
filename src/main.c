/* ovencontroller */

#define F_CPU 8000000L

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

#include "spi.h"




static int uart_putchar(char c, FILE *stream);

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL,
				_FDEV_SETUP_WRITE);


static int uart_putchar(char c, FILE *stream)
{
	if (c == '\n')
		uart_putchar('\r', stream);
	loop_until_bit_is_set(UCSRA, UDRE);
	UDR = c;
	return 0;
}


void init_uart(void)
{

	UBRRL = 8;
	UCSRB = (1<<TXEN);
	UCSRC = (1<<URSEL)|(3<<UCSZ0);

	stdout = &mystdout;
}



int main(void)
{
	unsigned char buf[4];
	short int_temp, ext_temp;

	init_uart();
	init_spi();

	while (1) {
		read_spi(TEMP_CS, 4, buf);

		int_temp = (buf[2] << 4) | (buf[3] >> 4);
		ext_temp = (buf[0] << 6) | (buf[1] >> 2);

		printf("int_temp=%d ext_temp=%d\n", int_temp>>4, ext_temp>>2);

		_delay_ms(1000);
	}

	return 0;
}

