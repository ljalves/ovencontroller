/* ovencontroller */

#define F_CPU 8000000L

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h> 
#include <util/delay.h>

#include "spi.h"
#include "uart.h"
#include "temp.h"
#include "lcd.h"


volatile unsigned char cnt250;
volatile unsigned char cnt10m;


ISR(TIMER0_OVF_vect)
{
	TCNT0 += 6;
	if (++cnt250 == 40) {
		cnt10m++;
		cnt250 = 0;
	}
}


void init_timer(void)
{
	/* Setup Timer 0 */
	/* prescaler/8 */
	TCCR0 = 1 << CS01;
	TCNT0 = 0;

	/* Enable timer0 interrupt */
	TIMSK |= (1<<TOIE0);
}



int main(void)
{

	struct temp_sensor temp;
	const char initmsg1[] = "Reflow Oven\0";
	const char initmsg2[] = "Controller 0v1\0";

	char buf[20];


	init_timer();
	init_uart();
	init_lcd();
	init_spi();

	lcd_sendline(LCD_LINE_1, initmsg1);
	lcd_sendline(LCD_LINE_2, initmsg2);
	_delay_ms(1500);



	/* Enable global interrupts */
	sei();

	while (1) {
		/* sec interval refresh */
		if (cnt10m > 100) {
			cnt10m = 0;
			read_temp(&temp);

			/* clear lcd */
			lcd_clear();

			sprintf(buf, "int_temp=%4d.%0004dC",
				temp.int_temp >> 4,
				((temp.ext_temp & 0xf) * 625));
			lcd_sendline(LCD_LINE_1, buf);
			printf("%s ", buf);

			sprintf(buf, "ext_temp=%4d.%02dC",
				temp.ext_temp >> 2,
				(temp.ext_temp & 0x3) * 25);
			lcd_sendline(LCD_LINE_2, buf);
			printf("%s\n", buf);

			printf("temp status: %s, %s, %s, %s\n",
				(temp.status & T_OPENCIRCUIT) ?
				"OPEN CIRCUIT" : "NO OPEN CIRCUIT",
				(temp.status & T_SHORTGND) ?
				"SHORT TO GND" : "NO SHORT TO GND",
				(temp.status & T_SHORTVCC) ?
				"SHORT TO VCC" : "NO SHORT TO VCC",
				(temp.status & T_FAULT) ? "FAULT" : "NO FAULT");
	
				
		}
	}
	return 0;
}

