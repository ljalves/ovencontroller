
#include <avr/interrupt.h> 
#include "timer.h"
#include "buzzer.h"


volatile unsigned char cnt250;
unsigned int volatile jiffies;

EXPORT_SYMBOL(jiffies);

ISR(TIMER0_OVF_vect)
{
	TCNT0 += 6;
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
	TCCR0 = 1 << CS01;
	TCNT0 = 0;

	/* Enable timer0 interrupt */
	TIMSK |= (1<<TOIE0);
}

//EXPORT_SYMBOL(jiffies);

