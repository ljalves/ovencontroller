/* controller */

#include "controller.h"
#include "temp.h"

#define CTRL_PID

#ifdef CTRL_IFTHENELSE
void controller(struct controller *c, struct temp_sensor *temp, int setpoint)
{
	if ((temp->ext_temp >> 2) < setpoint)
		ssr(1);
	else
		ssr(0);


}
#endif


#ifdef CTRL_PID
void controller(struct controller *c, struct temp_sensor *temp, int setpoint)
{
	int err, dinp;
	int inp = (temp->ext_temp >> 2);

	err = setpoint - inp;
	c->integral += err * c->ki;
	if (c->integral > c->max)
		c->integral = c->max;
	else if (c->integral < c->min)
		c->integral = c->min;

	dinp = inp - c->prev_input;
	c->output = c->kp * err + c->integral + c->kd * dinp;

	if (c->output > c->max)
		c->output = c->max;
	else if (c->output < c->min)
		c->output = c->min;


	c->prev_input = inp;
}

#endif
