#ifndef _BUTTONS_H_
#define _BUTTONS_H_

#include <avr/io.h>

#define BUTTONS_PORT  (PORTA)
#define BUTTONS_PIN1  (PA5)
#define BUTTONS_PIN2  (PA6)

#define BUTTONS_DPORT (DDRA)
#define BUTTONS_DPIN1 (DDA5)
#define BUTTONS_DPIN2 (DDA6)

typedef enum {
	BUTTONS_OFF = 0,
	BUTTONS_ON  = 1,
} buttons_state_t;

typedef enum {
	BUTTON1 = 0,
	BUTTON2 = 1,
} buttons_id_t;


void init_buttons(void);
buttons_state_t get_buttons_state(buttons_id_t id);

#endif
