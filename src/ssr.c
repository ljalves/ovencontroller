
#include <avr/io.h>
#include "ssr.h"

/* init solid state relay */
/* and turn if off */
void init_ssr(void)
{
	/* init SSR pin */
	/* set pin as output */
	SSR_DPORT |= 1 << SSR_DPIN;
	/* set pin low */
	set_ssr(SSR_OFF);
}

/* set solid state relay */
void set_ssr(ssr_state_t state)
{
	if (state == SSR_ON)
		SSR_PORT |= 1 << SSR_PIN;
	else
		SSR_PORT &= ~(1 << SSR_PIN);
}

