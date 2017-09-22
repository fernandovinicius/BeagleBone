#ifndef _GPIO_INTERFACE_H
#define _GPIO_INTERFACE_H
/***********************************************************************
 * INCLUDES
 **/
#include <stdint.h>

/***********************************************************************
 * FUNTIONS
 **/
int gpio_write(uint32_t gpio_num, uint8_t pin_level);
int gpio_read(uint32_t gpio_num, uint8_t *pin_level);

#endif
