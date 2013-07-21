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


int main(void)
{
	struct temp_sensor temp;
	struct controller ctrl = {
		.kp = 11, .kd = 0, .ki = 25,
		.integral = 0,
		.output = 0,
		.max = 500, .min = 0,
	};
	int last_temp, temp_rate;
	char sign;

	unsigned int time_now;
	unsigned int ps1_timer, ps2_timer, ps3_timer;



	unsigned int sec = 0;

	int setpoint = 100;




	unsigned char start, end;




	const char initmsg1[] = "Reflow Oven\0";
	const char initmsg2[] = "Controller 0v1\0";

	char buf[20];


	/* init peripherals */
	init_ssr();
	init_buzzer();
	init_spi();
	init_lcd();
	init_temp(&temp);
	init_timer();
	init_uart();

	/* LCD wellcome message */
	lcd_sendline(LCD_LINE_1, (char *) initmsg1);

	lcd_sendline(LCD_LINE_2, (char *) initmsg2);
	_delay_ms(1000);

	/* pre-fill previous temperature */
	ctrl.prev_input = temp.avg;
	last_temp = temp.avg;

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
	ps1_timer = ps2_timer = ps3_timer = now();
	for (;;) {
		time_now = now();

		/* control process - 200ms tick */
		if (time_now - ps1_timer > 10) {
			ps1_timer += 10;

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
		if (time_now - ps3_timer > 25) {
			ps3_timer += 25;

			/* run buzzer task */
			buzzer_task();
		}

		/* 1 sec tick */
		if (time_now - ps2_timer > 100) {
			ps2_timer += 100;
			sec++;

			temp_rate = temp.avg - last_temp;

			if ((temp.ext_temp >> 2 > 100) && (start == 0)) {
				start = 1;
				sec = 0;
			}

			if ((start == 1) && (end == 0)) {
				if (sec < 90) {
					setpoint = 150;
					//start_buzzer(ONE_BEEP_S);
				} else if ((sec < 120) || (temp.ext_temp >> 2 < 230)) {
					//start_buzzer(TWO_BEEPS_S);
					setpoint = 230;
				} else {
					start_buzzer(TWO_BEEPS_L);
					setpoint = 0;
					end = 1;
				}
			}

			/* clear lcd */
			lcd_clear();

			if (temp_rate < 0) {
				sign = '-';
				temp_rate = 0 - temp_rate;
			} else
				sign = ' ';

			sprintf(buf, "T=%3d.%02dC  Tr=%c%d.%02dC",
				temp.inst, (temp.avg & 3) * 25,
				sign, temp_rate >> 2, (temp_rate & 3) * 25);
			lcd_sendline(LCD_LINE_1, buf);
			printf("%s\n", buf);

			sprintf(buf, "Tset=%dC    PWM=%d",
				setpoint, ctrl.output);
			lcd_sendline(LCD_LINE_2, buf);
			printf("%s\n", buf);

			/* display total time */
			sprintf(buf, "Time=%02dm%02ds",
				sec / 60, sec % 60);
			lcd_sendline(LCD_LINE_3, buf);
			printf("%s\n", buf);


#ifdef DEBUG
			sprintf(buf, "int_temp=%4d.%04dC",
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


			printf("temp status: %s, %s, %s, %s\n",
				(temp.status & T_OPENCIRCUIT) ?
				"OPEN CIRCUIT" : "NO OPEN CIRCUIT",
				(temp.status & T_SHORTGND) ?
				"SHORT TO GND" : "NO SHORT TO GND",
				(temp.status & T_SHORTVCC) ?
				"SHORT TO VCC" : "NO SHORT TO VCC",
				(temp.status & T_FAULT) ? "FAULT" : "NO FAULT");
			

			sprintf(buf, "slope = %4d.%2d",
				(temp.ext_temp - last_temp) >> 2,
				((temp.ext_temp - last_temp) & 0x3) * 25);
			lcd_sendline(LCD_LINE_4, buf);
			printf("%s\n", buf);


#endif


			last_temp = temp.avg;
		}
	}
	return 0;
}

