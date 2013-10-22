
#include <avr/io.h>
#include "buttons.h"

/* init solid state relay */
/* and turn if off */
void init_buttons(void)
{
	/* init BUTTONS pins */
	/* set pins as input */
	BUTTONS_DPORT &= ~(1 << BUTTONS_DPIN1);
	BUTTONS_DPORT &= ~(1 << BUTTONS_DPIN2);
}

/* set solid state relay */
buttons_state_t get_buttons_state(buttons_id_t id)
{
	unsigned char buttons;

	if (id == BUTTON1)
		buttons = PINA & (1 << BUTTONS_PIN1);
	else
		buttons = PINA & (1 << BUTTONS_PIN2);

	if (buttons == 0)
		return BUTTONS_ON;
	else
		return BUTTONS_OFF;
}

