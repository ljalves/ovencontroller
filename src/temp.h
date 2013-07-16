#ifndef _TEMP_H_
#define _TEMP_H_

#include "spi.h"



typedef enum {
	T_OPENCIRCUIT = 0x01,
	T_SHORTGND    = 0x02,
	T_SHORTVCC    = 0x04,
	T_FAULT       = 0x08,
} temp_status_t;



struct temp_sensor {
	short int_temp;
	short ext_temp;
	unsigned char status;
};


void read_temp(struct temp_sensor *t);


#endif
