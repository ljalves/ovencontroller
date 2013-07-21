#ifndef _BUZZER_H_
#define _BUZZER_H_

#include <avr/io.h>


typedef enum {
	ONE_BEEP_L,
	TWO_BEEPS_L,
	ONE_BEEP_S,
	TWO_BEEPS_S,
} buzz_pattern_t;


typedef enum {
	IDLE,
	BEEP,
} buzz_state_t;


void buzzer_int(void);
void buzzer_task(void);
void init_buzzer(void);
void start_buzzer(buzz_pattern_t p);

#endif

