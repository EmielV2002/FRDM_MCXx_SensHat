#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#include <MCXA153.h>

//void potentiometer_init(void); //not necesary 
uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
int potentiometer_main(void);

#endif // POTENTIOMETER_H