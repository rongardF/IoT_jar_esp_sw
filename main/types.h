#ifndef MAIN_TYPES_H_
#define MAIN_TYPES_H_

#include "driver/gpio.h"

#define CONF_FLAG GPIO_NUM_12

#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_MAX_STA_CONN       1 // only one user can configure the device at any given time

#define NO_ACTIVITY_TIMEOUT_MS 300000 // time waited before going into deep-sleep because of no activity; 300s

#define CONF_IP_ADDR "192.168.1.10"
#define CONF_PORT_NUM 10000

#endif /* MAIN_TYPES_H_ */
