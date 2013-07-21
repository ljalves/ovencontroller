
#include <avr/io.h>
#include "ssr.h"
#include "timer.h"

struct ssr_config {
	ssr_state_t state;
	unsigned int period;
	unsigned int pwm;
	unsigned int start_time;
} ssr;

/* set ssr pin level */
static void set_ssr_pin(unsigned char level)
{
	if (level == 1)
		SSR_PORT |= 1 << SSR_PIN;
	else
		SSR_PORT &= ~(1 << SSR_PIN);
}

/* init solid state relay */
/* and turn if off */
void init_ssr(void)
{
	/* init SSR pin */
	/* set pin as output */
	SSR_DPORT |= 1 << SSR_DPIN;
	/* set pin low */
	set_ssr_pin(0);
	set_ssr_state(SSR_OFF);
}

/* set solid state relay */
void set_ssr_state(ssr_state_t state)
{
	ssr.state = state;
}

void set_ssr_period(unsigned int period)
{
	ssr.period = period;
}

void set_ssr_pwm(unsigned int pwm)
{
	ssr.pwm = pwm;
}


void ssr_task(void)
{
	unsigned int time_now;

	switch (ssr.state) {
	case SSR_OFF:
		set_ssr_pin(0);
		break;
	case SSR_ON:
		set_ssr_pin(1);
		break;
	case SSR_PWM:
		time_now = now();
		/* always on */
		if (ssr.pwm == ssr.period)
			set_ssr_pin(1);
		else
		/* compare current time with pwm value */
		if (time_now > ssr.pwm + ssr.start_time)
			set_ssr_pin(0);
		else
			set_ssr_pin(1);
		
		/* switch compare window */
		if (time_now > ssr.period + ssr.start_time)
			ssr.start_time += ssr.period;

		break;
	default:
		break;
	}
}

