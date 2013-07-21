
#include <avr/interrupt.h> 
#include "timer.h"


volatile unsigned char cnt250;

ISR(TIMER0_OVF_vect)
{
	TCNT0 += 6;
	if (++cnt250 == 40) {
		jiffies++;
		cnt250 = 0;
	}


/*	if (buzz > 0) {
		if (buzz_st == 0)
			PORTA |= 1 << PA4;
		else
			PORTA &= ~(1 << PA4);
		buzz_st = ~buzz_st;
	} else
		PORTA &= ~(1 << PA4);*/
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

