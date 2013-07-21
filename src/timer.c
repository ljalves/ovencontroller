
#include <avr/interrupt.h> 
#include "timer.h"
#include "buzzer.h"

/* counter that increments each 250us */
volatile unsigned char cnt250;

/* counter that increments each 10ms */
volatile unsigned int jiffies;


ISR(TIMER0_COMP_vect)
{
	if (++cnt250 == 40) {
		jiffies++;
		cnt250 = 0;
	}

	/* buzz if needed */
	buzzer_int();
}


void init_timer(void)
{
	/* Setup Timer 0 */
	/* prescaler/8 */
	TCCR0 = (1 << CS01) | (1 << WGM01);
	TCNT0 = 0;

	/* top value for the timer */
	OCR0 = 249;

	/* Enable timer0 interrupt */
	TIMSK |= (1 << OCIE0);
}

unsigned int now(void)
{
	unsigned int now;

	/* read the current value atomically */
	cli();
	now = jiffies;
	sei();
	return now;
}

