
#include <stdio.h>
#include <avr/io.h>
#include "uart.h"




static int uart_putchar(char c, FILE *stream)
{
	if (c == '\n')
		uart_putchar('\r', stream);
	loop_until_bit_is_set(UCSRA, UDRE);
	UDR = c;
	return 0;
}


static FILE uartstdout = FDEV_SETUP_STREAM(uart_putchar, NULL,
				_FDEV_SETUP_WRITE);


void init_uart(void)
{
	/* baud rate select register */
	UBRRL = 8;

	/* TX enabled */
	UCSRB = (1<<TXEN);

	/* 8 bit, no parity, 1 stop bit */
	UCSRC = (1<<URSEL)|(3<<UCSZ0);

	/* redirect stdout to the uart */
	stdout = &uartstdout;
}


