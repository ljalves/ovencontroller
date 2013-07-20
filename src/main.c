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
#include "controller.h"

volatile unsigned char cnt250;
volatile unsigned char cnt10m, cnt_w, buzz = 1, buzz_st = 0;


volatile unsigned char cb = 0;


ISR(TIMER0_OVF_vect)
{
	TCNT0 += 6;
	if (++cnt250 == 40) {
		cnt_w++;
		cnt10m++;
		cnt250 = 0;
	}

//	if (++cb == 1) {
//		cb = 0;

		if (buzz > 0) {
			if (buzz_st == 0)
				PORTA |= 1 << PA4;
			else
				PORTA &= ~(1 << PA4);
			buzz_st = ~buzz_st;
		} else
			PORTA &= ~(1 << PA4);
//	}
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

void init_misc(void)
{
	/* init SSR pin */
	/* set pin as output */
	DDRA |= 1 << DDA7;
	/* set pin low */
	PORTA &= ~(1 << PA7);


	/* init buzzer */
	/* pin as output */
	DDRA |= 1 << DDA4;
	/* set pin low */
	PORTA &= ~(1 << PA4);
}

static void ssr(unsigned char state)
{
	if (state == 1)
		PORTA |= 1 << PA7;
	else
		PORTA &= ~(1 << PA7);
}



int main(void)
{
	struct temp_sensor temp;
	struct controller ctrl = {
		.kp = 11, .kd = 1, .ki = 45,
		.integral = 0,
		.output = 0,
		.max = 5000, .min = 0,
	};

	unsigned int window = 0;
	unsigned int now = 0;

	unsigned int sec = 0;

	int setpoint = 100;


	unsigned char start, end;


	const char initmsg1[] = "Reflow Oven\0";
	const char initmsg2[] = "Controller 0v1\0";

	char buf[20];

	init_timer();
	init_uart();
	init_lcd();
	init_spi();
	init_misc();


	lcd_sendline(LCD_LINE_1, initmsg1);
	lcd_sendline(LCD_LINE_2, initmsg2);
	_delay_ms(1500);


	read_temp(&temp);
	ctrl.prev_input = temp.ext_temp >> 2;

	/* Enable global interrupts */
	sei();

	start = 0;
	end = 0;
	while (1) {

		/* 100ms tick */
		if (cnt_w > 10) {
			cnt_w = 0;

			now += 100;
			if (now - window > 5000)
				window += 5000;

			read_temp(&temp);

			/* call controller */
			controller(&ctrl, &temp, setpoint);

			if (ctrl.output > now - window)
				ssr(1);
			else
				ssr(0);
		
		}



		

		/* 1 sec tick */
		if (cnt10m > 100) {
			cnt10m = 0;
			sec++;

			if (buzz > 0)
				buzz--;

			if ((temp.ext_temp >> 2 > 100) && (start == 0)) {
				start = 1;
				sec = 0;
			}

			if ((start == 1) && (end == 0)) {
				/* calc setpoint */
				if (sec < 90) {
					setpoint = 170;
				} else if ((sec < 120) || (temp.ext_temp >> 2 < 230)) {
					setpoint = 230;
				} else {
					setpoint = 0;
					end = 1;
					buzz = 2;
				}
			}


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

			sprintf(buf, "output=%4d",
				ctrl.output);
			lcd_sendline(LCD_LINE_3, buf);
			printf("%s\n", buf);

			sprintf(buf, "setpoint=%4d",
				setpoint);
			lcd_sendline(LCD_LINE_4, buf);
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

