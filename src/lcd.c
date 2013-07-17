
#define F_CPU 8000000L

#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"



void lcd_write(unsigned char b, lcd_reg_t reg)
{
	_delay_us(100);
	/* set write */
	PORTE &= ~(1 << LCD_RW);

	/* set port as output */
	DDRC = 0xff;

	/* send byte to lcd data pins */
	PORTC = b;

	/* raise en */
	PORTE |= 1 << LCD_EN;

	/* set reg mode */
	switch (reg) {
	case LCD_INSTREG:
		PORTE &= ~(1 << LCD_RS);
		break;
	case LCD_DATAREG:
		PORTE |= 1 << LCD_RS;
		break;
	}

	/* latch data */
	_delay_us(400);
	PORTE &= ~(1 << LCD_EN);
	
	_delay_us(200);

	/* set read */
	PORTE |= 1 << LCD_RW;
}




void init_lcd(void)
{
	/* turn on backlight */
	DDRD |= 1 << DDD5;
	PORTD |= 1 << PD5;

	/* init portc as input */
	DDRC = 0x00;

	/* rw, ds, en pins as output */
	DDRE = 0x07;

	/* set read as default mode*/
	PORTE = ~(1 << LCD_RW);


	_delay_ms(100);

	lcd_write(0x38, LCD_INSTREG);
	_delay_ms(400);
	lcd_write(0x38, LCD_INSTREG);
	_delay_ms(200);
	lcd_write(0x38, LCD_INSTREG);
	_delay_ms(200);
	lcd_write(0x0c, LCD_INSTREG);
	lcd_write(0x01, LCD_INSTREG);
	lcd_write(0x06, LCD_INSTREG);

}


void lcd_clear(void)
{
	lcd_write(0x01, LCD_INSTREG);
}


void lcd_sendline(unsigned char line, char *buf)
{
	lcd_write(line, LCD_INSTREG);

	while (*buf)
		lcd_write(*buf++, LCD_DATAREG);

}



