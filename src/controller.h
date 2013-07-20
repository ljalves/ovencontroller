#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_


#include "temp.h"

struct controller {
	int kp, kd, ki;
	int prev_input;
	int integral;
	int output;

	int max, min;
};

void controller(struct controller *c, struct temp_sensor *temp, int setpoint);


#endif
