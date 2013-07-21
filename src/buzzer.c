
#include "buzzer.h"


#define BUZZER_PORT  (PORTA)
#define BUZZER_DPORT (DDRA)

#define BUZZER_PIN   (PA4)
#define BUZZER_DPIN  (DDA4)

struct buzzer_config {
	unsigned char state;
	unsigned char seq[6];
	unsigned char len;
	unsigned char idx;

	unsigned char pin;
} buzzer;


static void set_buzzer_pin(unsigned int level)
{
	if (level)
		BUZZER_PORT |= 1 << BUZZER_PIN;
	else
		BUZZER_PORT &= ~(1 << BUZZER_PIN);

}

void buzzer_int(void)
{
	switch (buzzer.state) {
	case IDLE:
		set_buzzer_pin(0);
		break;
	case BEEP:
		set_buzzer_pin(buzzer.pin);
		buzzer.pin = ~buzzer.pin;
		break;
	default:
		buzzer.state = IDLE;
	}
}

void buzzer_task(void)
{
	/* no more sound to play */
	if (buzzer.len == 0) {
		buzzer.state = IDLE;
		return;
	}

	/* turn sound on/off */
	if (buzzer.seq[buzzer.idx] == 1)
		buzzer.state = BEEP;
	else
		buzzer.state = IDLE;

	/* deal with the buffer */
	buzzer.idx++;
	buzzer.len--;
}


/* init buzzer */
void init_buzzer(void)
{
	/* pin as output */
	BUZZER_DPORT |= 1 << BUZZER_DPIN;
	/* set pin low */
	set_buzzer_pin(0);

	buzzer.pin = 0;
	buzzer.state = IDLE;
}


void start_buzzer(buzz_pattern_t p)
{
	buzzer.idx = 0;

	switch (p) {
	case ONE_BEEP_L:
		buzzer.seq[0] = 1;
		buzzer.seq[1] = 1;
		buzzer.len = 2;
		break;
	case ONE_BEEP_S:
		buzzer.seq[0] = 1;
		buzzer.len = 1;
		break;
	case TWO_BEEPS_L:
		buzzer.seq[0] = 1;
		buzzer.seq[1] = 1;
		buzzer.seq[2] = 0;
		buzzer.seq[3] = 1;
		buzzer.seq[4] = 1;
		buzzer.len = 5;
		break;
	case TWO_BEEPS_S:
		buzzer.seq[0] = 1;
		buzzer.seq[1] = 0;
		buzzer.seq[2] = 1;
		buzzer.len = 3;
		break;
	default:
		buzzer.state = IDLE;
		return;
	}
}

