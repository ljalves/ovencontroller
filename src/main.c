/* ovencontroller */

#define F_CPU 8000000L

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h> 
#include <util/delay.h>

#include "ssr.h"
#include "spi.h"
#include "uart.h"
#include "timer.h"
#include "buzzer.h"

#include "temp.h"
#include "lcd.h"
#include "controller.h"

void main(void)
{
	struct temp_sensor temp;
	struct controller ctrl = {
		.kp = 11, .kd = 1, .ki = 45,
		.integral = 0,
		.output = 0,
		.max = 500, .min = 0,
	};

	unsigned int sec = 0;

	int setpoint = 100;

	int last_temp = 0;


	unsigned char start, end;


	unsigned int ps1_timer, ps2_timer;
	unsigned int ps3_timer;


	const char initmsg1[] = "Reflow Oven\0";
	const char initmsg2[] = "Controller 0v1\0";

	char buf[20];


	/* init peripherals */
	init_ssr();
	init_buzzer();
	init_spi();
	init_lcd();
	init_timer();
	init_uart();



	lcd_sendline(LCD_LINE_1, initmsg1);
	lcd_sendline(LCD_LINE_2, initmsg2);
	_delay_ms(1500);


	read_temp(&temp);
	ctrl.prev_input = temp.ext_temp >> 2;


	/* setup ssr pwm controller */
	set_ssr_period(500);
	set_ssr_pwm(250);
	set_ssr_state(SSR_PWM);


	/* Enable global interrupts */
	sei();

	start = 0;
	end = 0;



	/* ready beep */
	start_buzzer(ONE_BEEP_L);

	/* main task loop */
	ps1_timer = ps2_timer = ps3_timer = jiffies;
	for (;;) {

		/* control process - 200ms tick */
		if (jiffies - ps1_timer > 20) {
			ps1_timer += 20;

			/* read temperature */
			read_temp(&temp);

			/* call controller */
			controller(&ctrl, &temp, setpoint);

			/* set ssr pwm output value */
			set_ssr_pwm(ctrl.output);

			/* run ssr task */
			ssr_task();
		}

		/* buzzer process - 250ms tick */
		if (jiffies - ps3_timer > 25) {
			ps3_timer += 25;

			/* run buzzer task */
			buzzer_task();
		}

		/* 1 sec tick */
		if (jiffies - ps2_timer > 100) {
			ps2_timer += 100;
			sec++;

			if ((temp.ext_temp >> 2 > 100) && (start == 0)) {
				start = 1;
				sec = 0;
			}

			if ((start == 1) && (end == 0)) {
				if (sec < 90) {
					setpoint = 150;
					start_buzzer(ONE_BEEP_S);
				} else if ((sec < 120) || (temp.ext_temp >> 2 < 230)) {
					start_buzzer(TWO_BEEPS_S);
					setpoint = 230;
				} else {
					start_buzzer(TWO_BEEPS_L);
					setpoint = 0;
					end = 1;
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

			sprintf(buf, "pwm=%4d sp=%3d",
				ctrl.output, setpoint);
			lcd_sendline(LCD_LINE_3, buf);
			printf("%s\n", buf);


			/*printf("temp status: %s, %s, %s, %s\n",
				(temp.status & T_OPENCIRCUIT) ?
				"OPEN CIRCUIT" : "NO OPEN CIRCUIT",
				(temp.status & T_SHORTGND) ?
				"SHORT TO GND" : "NO SHORT TO GND",
				(temp.status & T_SHORTVCC) ?
				"SHORT TO VCC" : "NO SHORT TO VCC",
				(temp.status & T_FAULT) ? "FAULT" : "NO FAULT");
			*/

			sprintf(buf, "slope = %4d.%2d",
				(temp.ext_temp - last_temp) >> 2,
				((temp.ext_temp - last_temp) & 0x3) * 25);
			lcd_sendline(LCD_LINE_4, buf);
			printf("%s\n", buf);

			last_temp = temp.ext_temp;


			printf("jiffies=%d ps2_timer=%d\n", jiffies, ps2_timer);
		}
	}
}

