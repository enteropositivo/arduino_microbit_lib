
#ifndef mbit_utils_h
#define mbit_utils_h

#include <Arduino.h>


extern uint16_t mbit_utils_analog_read(uint32_t ulPin );

extern uint16_t mbit_utils_analog_int(uint32_t ulPin );

extern void ADC_IRQHandler(void);

#endif
