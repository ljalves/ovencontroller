#ifndef _SSR_H_
#define _SSR_H_

#include <avr/io.h>

#define SSR_PORT  (PORTA)
#define SSR_PIN   (PA7)

#define SSR_DPORT (DDRA)
#define SSR_DPIN  (DDA7)

typedef enum {
	SSR_OFF,
	SSR_ON,
	SSR_PWM,
} ssr_state_t;


void init_ssr(void);
void set_ssr_period(unsigned int period);
void set_ssr_pwm(unsigned int pwm);
void set_ssr_state(ssr_state_t state);
void ssr_task(void);


#endif
