#ifndef _TIMER_H_
#define _TIMER_H_


volatile unsigned long jiffies;
EXPORT_SYMBOL(jiffies);

void init_timer(void);


#endif
