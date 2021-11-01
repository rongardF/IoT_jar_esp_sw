
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

volatile EventGroupHandle_t s_sta_ap_event_group;

void wifi_init_sta_ap();

#endif /* WIFI_IOT_H_ */
