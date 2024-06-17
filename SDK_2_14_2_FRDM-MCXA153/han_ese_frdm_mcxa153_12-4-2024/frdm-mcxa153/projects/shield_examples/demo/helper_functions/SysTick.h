#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

extern volatile uint32_t ms;
extern volatile uint32_t systick_flag;

void SysTick_Handler(void)
{
  ms++;
  systick_flag = 1;
};

#endif /* SYSTICK_H */
