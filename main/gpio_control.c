#include "gpio_control.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "driver/gpio.h"

void setup_GPIO(gpio_int_type_t mode, uint32_t pbm, gpio_pulldown_t pde, gpio_pullup_t pue)
{
	static gpio_config_t io_conf;

	io_conf.intr_type = GPIO_INTR_DISABLE; //disable interrupt
	io_conf.mode = mode;
	io_conf.pin_bit_mask = pbm;  //bit mask of the pins that you want to set,e.g.GPIO15/16
	io_conf.pull_down_en = pde; //disable pull-down mode
	io_conf.pull_up_en = pue;  //disable pull-up mode

	gpio_config(&io_conf); //configure GPIO with the given settings
}
