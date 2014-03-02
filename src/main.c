/* ovencontroller */

#define F_CPU 8000000L

#undef DEBUG

#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h> 
#include <util/delay.h>

#include "ssr.h"
#include "spi.h"
#include "uart.h"
#include "timer.h"
#include "buzzer.h"
#include "buttons.h"

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


struct reflow_profile {
	unsigned int preheat_temp;
	unsigned int preheat_time;

	unsigned int soak_temp;
	unsigned int soak_time;

	unsigned int reflow_temp;
	unsigned int reflow_time;

	unsigned int peak_temp;
	unsigned int peak_time;
} reflow_profiles[] = {
	{
		.preheat_temp = 100,
		.preheat_time = 120,
		.soak_temp = 140,
		.soak_time = 90,
		.reflow_temp = 225,
		.reflow_time = 30,
		.peak_temp = 230,
		.peak_time = 10,
	},
	{
		.preheat_temp = 120,
		.preheat_time = 120,
		.soak_temp = 170,
		.soak_time = 90,
		.reflow_temp = 235,
		.reflow_time = 30,
		.peak_temp = 240,
		.peak_time = 10,
	},
};
#if 1
const struct state_names_s {
	char name[4];
} state_names[] = {
	{ .name = "IDL"},
	{ .name = "PRE"},
	{ .name = "SOA"},
	{ .name = "REF"},
	{ .name = "PEA"},
	{ .name = "COO"},
};
#endif

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

	struct reflow_profile *p;

	unsigned int time_now;
	unsigned int ps1_timer, ps2_timer, ps3_timer;

	/* main state machine */
	main_state_t main_state;

	unsigned int total_sec = 0, sec = 0, show_profile = 0;

	int setpoint = 0;

	unsigned char sw1 = 0, sw2 = 0, reflow_profile_idx = 0;

	const char initmsg1[] = "Reflow Oven";
	const char initmsg2[] = "Controller 0v2";

	char buf[25];


	init_spi();
	init_lcd();
	/* LCD wellcome message */
	lcd_sendline(LCD_LINE_1, (char *) initmsg1);
	lcd_sendline(LCD_LINE_2, (char *) initmsg2);

	init_timer();
#ifdef DEBUG
	init_uart();
#endif
	init_ssr();
	init_buzzer();
	init_buttons();

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

	/* Initial state = IDLE */
	main_state = ST_IDLE;

	/* init process timers */
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

			/* check buttons */
			if (sw1 == BUTTONS_OFF)
				sw1 = get_buttons_state(BUTTON1);
			if (sw2 == BUTTONS_OFF)
				sw2 = get_buttons_state(BUTTON2);
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

			if (show_profile > 0)
				show_profile--;

			p = &reflow_profiles[reflow_profile_idx];

			/* calculate temperature rate */
			temp_rate = temp.avg - last_temp;

			switch (main_state) {
			case ST_IDLE:

				if (sw1 == BUTTONS_ON) {
					show_profile = 5;
					start_buzzer(ONE_BEEP_S);
					reflow_profile_idx++;
					if (reflow_profile_idx >=
						(sizeof(reflow_profiles) /
						 sizeof(reflow_profiles[0])))
						reflow_profile_idx = 0;
					sw1 = BUTTONS_OFF;

					p = &reflow_profiles[reflow_profile_idx];
				}

				if (sw2 == BUTTONS_ON) {
					main_state = ST_PREHEAT;
					start_buzzer(ONE_BEEP_S);
					setpoint = p->preheat_temp;
				} else {
					/* turn off SSR */
					setpoint = 0;		
				}
				/* init seconds timer */
				total_sec = sec = 0;

				break;
			case ST_PREHEAT:
				if ((sec > p->preheat_time) &&
				    (temp.avg > (p->preheat_temp << 2))) {
					setpoint = p->soak_temp;
					main_state = ST_SOAK;
					start_buzzer(ONE_BEEP_S);
					sec = 0;
				}
				break;
			case ST_SOAK:
				if ((sec > p->soak_time) &&
				    (temp.avg > (p->soak_temp << 2))) {
					setpoint = p->reflow_temp;
					main_state = ST_REFLOW;
					start_buzzer(TWO_BEEPS_S);
					sec = 0;
				}
				break;
			case ST_REFLOW:
				if ((sec > p->reflow_time) &&
				    (temp.avg > (p->reflow_temp << 2))) {
					setpoint = p->peak_temp;
					main_state = ST_PEAK;
					start_buzzer(ONE_BEEP_L);
					sec = 0;
				}
				break;
			case ST_PEAK:
				if (sec > p->peak_time) {
					main_state = ST_COOLDOWN;
					start_buzzer(TWO_BEEPS_L);
					setpoint = 0;
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

			if (sw1 == BUTTONS_ON) {
				main_state = ST_IDLE;
				start_buzzer(TWO_BEEPS_S);
				sw1 = BUTTONS_OFF;
			}
			if (sw2 == BUTTONS_ON) {
				sw2 = BUTTONS_OFF;
			}

#if 1
			/* clear lcd */
			//lcd_clear();


			if (show_profile) {
				sprintf(buf, "Preheat %3ds @ %3dC ",
					p->preheat_time,
					p->preheat_temp);
				lcd_sendline(LCD_LINE_1, buf);
				sprintf(buf, "Soak    %3ds @ %3dC ",
					p->soak_time,
					p->soak_temp);
				lcd_sendline(LCD_LINE_2, buf);
				sprintf(buf, "Reflow  %3ds @ %3dC ",
					p->reflow_time,
					p->reflow_temp);
				lcd_sendline(LCD_LINE_3, buf);
				sprintf(buf, "Peak    %3ds @ %3dC ",
					p->peak_time,
					p->peak_temp);
				lcd_sendline(LCD_LINE_4, buf);



			} else {

				if (temp_rate < 0) {
					sign = '-';
					temp_rate = 0 - temp_rate;
				} else
					sign = ' ';

				sprintf(buf, "T=%3d.%02dC R=%c%1d.%02dC/s",
					temp.inst, (temp.avg & 3) * 25,
					sign, temp_rate >> 2, (temp_rate & 3) * 25);
				lcd_sendline(LCD_LINE_1, buf);
				//printf("%s\n", buf);

				sprintf(buf, "Tset=%3dC   PWM=%3d",
					setpoint, ctrl.output);
				lcd_sendline(LCD_LINE_2, buf);
				//printf("%s\n", buf);

				/* display total time */
				sprintf(buf, "Time=%02dm%02ds  P=%d   ",
					total_sec / 60, total_sec % 60,
					reflow_profile_idx);
				lcd_sendline(LCD_LINE_3, buf);
				//printf("%s\n", buf);

				sprintf(buf, "State: %s         ", state_names[main_state].name);
				//sprintf(buf, "");
				lcd_sendline(LCD_LINE_4, buf);
				//printf("%s %d %d\n", buf);

			}
#endif		

#ifdef DEBUG
			sprintf(buf, "int_temp=%4d.%04dC",
				temp.int_temp >> 4,
				((temp.ext_temp & 0xf) * 625));
//			lcd_sendline(LCD_LINE_1, buf);
			printf("%s ", buf);

			sprintf(buf, "ext_temp=%4d.%02dC",
				temp.ext_temp >> 2,
				(temp.ext_temp & 0x3) * 25);
//			lcd_sendline(LCD_LINE_2, buf);
			printf("%s\n", buf);

			sprintf(buf, "pwm=%4d sp=%3d",
				ctrl.output, setpoint);
//			lcd_sendline(LCD_LINE_3, buf);
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
//			lcd_sendline(LCD_LINE_4, buf);
			printf("%s\n", buf);
#endif


			last_temp = temp.avg;
		}
#endif
	}
	return 0;
}

