#ifndef _TEMP_H_
#define _TEMP_H_

#include "spi.h"

/* temperature averages */
/* number of averages = 2^NR_AVG */
#define NR_AVG 2

typedef enum {
	T_OPENCIRCUIT = 0x01,
	T_SHORTGND    = 0x02,
	T_SHORTVCC    = 0x04,
	T_FAULT       = 0x08,
} temp_status_t;


struct temp_sensor {
	/* readings from sensor */
	int int_temp;
	int ext_temp;
	unsigned char status;

	/* calculated */
	int inst, avg;
	int buf[1 << NR_AVG];
	unsigned char idx;
};


void init_temp(struct temp_sensor *t);
void read_temp(struct temp_sensor *t);


#endif
