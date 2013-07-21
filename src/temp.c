#include "temp.h"
#include "spi.h"

#include <stdio.h>

void init_temp(struct temp_sensor *t)
{
	unsigned char i;

	t->idx = 0;
	read_temp(t);

	/* pre-fill average buffer */
	for (i = 0; i < (1 << NR_AVG); i++)
		t->buf[i] = t->ext_temp;

	read_temp(t);
}

void read_temp(struct temp_sensor *t)
{
	unsigned char buf[4];
	unsigned char i;
	unsigned int sum = 0;

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


	/* insert in buffer */
	t->buf[t->idx++] = t->ext_temp;

	/* trim index */
	t->idx &= (1 << NR_AVG) - 1;

	/* calc averave */
	for (i = 0; i < (1 << NR_AVG); i++)
		sum += t->buf[i];
	t->avg = sum >> NR_AVG;

	/* rounded to integer */
	t->inst = t->avg >> 2;
}

