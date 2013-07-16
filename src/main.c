/* ovencontroller */

#define F_CPU 8000000L

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#include "spi.h"
#include "uart.h"
#include "temp.h"



int main(void)
{
	struct temp_sensor temp;

	init_uart();
	init_spi();

	while (1) {
		read_temp(&temp);

		printf("int_temp=%d ext_temp=%d\n",
			temp.int_temp>>4, temp.ext_temp>>2);

		printf("temp status: %s, %s, %s, %s\n",
			(temp.status & T_OPENCIRCUIT) ? "OPEN CIRCUIT" : "NO OPEN CIRCUIT",
			(temp.status & T_SHORTGND) ? "SHORT TO GND" : "NO SHORT TO GND",
			(temp.status & T_SHORTVCC) ? "SHORT TO VCC" : "NO SHORT TO VCC",
			(temp.status & T_FAULT) ? "FAULT" : "NO FAULT");


		_delay_ms(1000);
	}
	return 0;
}

