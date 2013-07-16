#include "temp.h"
#include "spi.h"


void read_temp(struct temp_sensor *t)
{
	unsigned char buf[4];

	/* read 4 bytes from the SPI bus */
	read_spi(TEMP_CS, 4, buf);

	t->int_temp = (buf[2] << 4) | (buf[3] >> 4);
	/* sign extend if negative */
	if (t->int_temp & 0x0400)
		t->int_temp |= 0xF000;

	t->ext_temp = (buf[0] << 6) | (buf[1] >> 2);
	/* sign extend if negative */
	if (t->ext_temp & 0x2000)
		t->ext_temp |= 0xC000;

	t->status = (buf[3] & 0x07) | ((buf[1] & 0x01) << 3);
}

