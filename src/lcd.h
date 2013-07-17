#ifndef _LCD_H_
#define _LCD_H_

#include <avr/io.h>


#define LCD_LINE_1 (0x80)
#define LCD_LINE_2 (0xc0)
#define LCD_LINE_3 (0x94)
#define LCD_LINE_4 (0xd4)


/* pins */
#define LCD_EN 0
#define LCD_RS 1
#define LCD_RW 2


typedef enum {
	LCD_INSTREG = 0,
	LCD_DATAREG = 1,
} lcd_reg_t;

void init_lcd(void);
void lcd_sendline(unsigned char line, char *buf);
void lcd_clear(void);



#endif

