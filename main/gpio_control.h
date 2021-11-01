
#ifndef MAIN_GPIO_CONTROL_H_
#define MAIN_GPIO_CONTROL_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "driver/gpio.h"

#define CONF_FLAG GPIO_NUM_2

void setup_GPIO(gpio_int_type_t mode, uint32_t pbm, gpio_pulldown_t pde, gpio_pullup_t pue);

#endif /* MAIN_GPIO_CONTROL_H_ */

