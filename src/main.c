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

typedef enum {
	ST_IDLE,
	ST_PREHEAT,
	ST_SOAK,
	ST_REFLOW,
	ST_PEAK,
	ST_COOLDOWN,
} main_state_t;

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

	/* main state machine */
	main_state_t main_state;

	unsigned int total_sec, sec;

	int setpoint = 100;


	const char initmsg1[] = "Reflow Oven\0";
	const char initmsg2[] = "Controller 0v1\0";


	char buf[25];


	init_spi();
	init_lcd();
	/* LCD wellcome message */
	lcd_sendline(LCD_LINE_1, (char *) initmsg1);
	lcd_sendline(LCD_LINE_2, (char *) initmsg2);

	init_timer();
	init_uart();
	init_ssr();
	init_buzzer();


	init_temp(&temp);

	/* pre-fill previous temperature */
	ctrl.prev_input = temp.avg;
	last_temp = temp.avg;

	/* setup ssr pwm controller */
	set_ssr_period(500);
	set_ssr_pwm(250);
	set_ssr_state(SSR_PWM);


	_delay_ms(500);

	/* Enable global interrupts */
	sei();

	/* ready beep */
	start_buzzer(ONE_BEEP_L);

	/* TODO: add button control */
	main_state = ST_PREHEAT;
	total_sec = sec = 0;

	/* init process timers and 'un-phase' them*/
	ps1_timer = ps2_timer = ps3_timer = now();

	/* main loop */
	for (;;) {
		time_now = now();

		/* control process - 100ms tick */
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
#if 1
		/* buzzer process - 250ms tick */
		if (time_now - ps3_timer > 25) {
			ps3_timer += 25;

			/* run buzzer task */
			buzzer_task();
		}
#endif
#if 1
		/* main state machine - 1 sec tick */
		if (time_now - ps2_timer > 100) {
			ps2_timer += 100;
			total_sec++;
			sec++;

			/* calculate temperature rate */
			temp_rate = temp.avg - last_temp;


			switch (main_state) {
			case ST_IDLE:
				break;
			case ST_PREHEAT:
				if (temp.avg > (100 << 2)) {
					setpoint = 140;
					main_state = ST_SOAK;
					start_buzzer(ONE_BEEP_S);
					sec = 0;
				}
				break;
			case ST_SOAK:
				if ((sec > 90) && (temp.avg > (140 << 2))) {
					setpoint = 225;
					main_state = ST_REFLOW;
					start_buzzer(TWO_BEEPS_S);
					sec = 0;
				}
				break;
			case ST_REFLOW:
				if ((sec > 30) && (temp.avg > (225 << 2))) {
					setpoint = 0;
					main_state = ST_PEAK;
					start_buzzer(ONE_BEEP_L);
					sec = 0;
				}
				break;
			case ST_PEAK:
				if (sec > 10) {
					main_state = ST_COOLDOWN;
					start_buzzer(TWO_BEEPS_L);
				}
				break;
			case ST_COOLDOWN:
				if (temp.avg < (60 << 2)) {
					main_state = ST_IDLE;
					start_buzzer(TWO_BEEPS_S);
				}
				break;
			default:
				main_state = ST_IDLE;
				break;
			}

#if 1
			/* clear lcd */
			lcd_clear();

			if (temp_rate < 0) {
				sign = '-';
				temp_rate = 0 - temp_rate;
			} else
				sign = ' ';

			sprintf(buf, "T=%3d.%02dC R=%c%d.%02dC/s",
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
				total_sec / 60, total_sec % 60);
			lcd_sendline(LCD_LINE_3, buf);
			printf("%s\n", buf);


			sprintf(buf, "State: %s",
				(main_state == ST_IDLE) ? "IDLE" :
				(main_state == ST_PREHEAT) ? "PREHEAT" :
				(main_state == ST_SOAK) ? "SOAK" :
				(main_state == ST_REFLOW) ? "REFLOW" :
				(main_state == ST_PEAK) ? "PEAK" :
				(main_state == ST_COOLDOWN) ? "COOLDOWN" : "?");
			lcd_sendline(LCD_LINE_4, buf);
			printf("%s\n", buf);
#endif		

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
#endif
	}
	return 0;
}

