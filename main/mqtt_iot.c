#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"

#include "mqtt_client.h"
#include "mqtt_iot.h"
#include "esp_log.h"

static void mqtt_event_handler_cb (esp_mqtt_event_handle_t event)
{
	esp_mqtt_client_handle_t client = event->client;
	int msg_id;
	// your_context_t *context = event->context;
	switch (event->event_id) {
		case MQTT_EVENT_CONNECTED:
			ESP_LOGI("MQTT_EVENT_HANDLER", "MQTT_EVENT_CONNECTED");
			msg_id = esp_mqtt_client_publish(client, "main", "TEST", 0, 1, 0);
			ESP_LOGI("MQTT_EVENT_HANDLER", "Sent publish successful, msg_id=%d", msg_id);
			break;
		case MQTT_EVENT_DISCONNECTED:
			ESP_LOGI("MQTT_EVENT_HANDLER", "MQTT_EVENT_DISCONNECTED");
			break;
//		case MQTT_EVENT_SUBSCRIBED:
//			ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
//			msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
//			ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
//			break;
//		case MQTT_EVENT_UNSUBSCRIBED:
//			ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
//			break;
		case MQTT_EVENT_PUBLISHED:
			ESP_LOGI("MQTT_EVENT_HANDLER", "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
			break;
		case MQTT_EVENT_DATA:
			ESP_LOGI("MQTT_EVENT_HANDLER", "MQTT_EVENT_DATA");
			printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
			printf("DATA=%.*s\r\n", event->data_len, event->data);
			break;
		case MQTT_EVENT_ERROR:
			ESP_LOGI("MQTT_EVENT_HANDLER", "MQTT_EVENT_ERROR");
			break;
		case MQTT_EVENT_BEFORE_CONNECT:
			ESP_LOGI("MQTT_EVENT_HANDLER", "MQTT_EVENT_BEFORE_CONNECT");
			break;
		default:
			ESP_LOGI("MQTT_EVENT_HANDLER", "Other event id:%d", event->event_id);
			break;
	}
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD("MQTT_EVENT_HANDLER", "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

void mqtt_task()
{
	ESP_LOGI("MAIN","Creating MQTT config structure");

	esp_mqtt_client_config_t mqtt_cfg = {
			.host = MQTT_BROKER_IP,
			.port = MQTT_BROKER_PORT,
			.transport= MQTT_TRANSPORT_OVER_TCP
		};

	ESP_LOGI("MAIN","Creating MQTT handle");
	esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
	ESP_LOGI("MAIN","Registering MQTT events to event loop");
	esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
	ESP_LOGI("MAIN","Starting MQTT");
	esp_mqtt_client_start(client);
	ESP_LOGI("MAIN","Main has finished");

	while (1)
	{
		vTaskDelay(5000 / portTICK_RATE_MS);
	}
}
