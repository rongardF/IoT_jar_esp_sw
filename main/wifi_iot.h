
#ifndef MAIN_WIFI_IOT_H_
#define MAIN_WIFI_IOT_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "freertos/event_groups.h"

#define AP_WIFI_SSID "myTest"
#define AP_WIFI_PASS "myPassword"
#define AP_MAX_CONNECTIONS 1
#define STA_CONNECTED BIT0
#define CONFIGURED_TO_APSTA BIT1
#define CONF_IP_ADDR "192.168.1.10"
#define CONF_PORT_NUM 10000

volatile EventGroupHandle_t s_sta_ap_event_group;

void wifi_init_ap();
void reconfigure_wifi_to_apsta(uint8_t * ssid, uint8_t * password);

#endif /* WIFI_IOT_H_ */
