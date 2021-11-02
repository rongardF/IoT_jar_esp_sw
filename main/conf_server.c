#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "conf_server.h"
#include "wifi_iot.h"

void conf_server()
{

}

// this creates a server which accepts connection from the user to configure the device
void listener (){
	struct sockaddr_in server_addr, client_addr;
	int32_t sockfd, user_socket;
	char message[200];

	if (( sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		// log and print the error
		ESP_LOGE("CONF_SERVER", "Server listener failed to get the socket handler\r\n");
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY; // convert IP address string to long format and assign it into IP address variable in structure
	server_addr.sin_port = htons(CONF_PORT_NUM);

	if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
	{
		// log and print the error
		ESP_LOGE("CONF_SERVER", "Failed to bind to socket\r\n");
	}

	if (listen(sockfd, 1) < 0) // listen for incoming connections; only one connections can be in a queue
	{
		// log and print the error
		ESP_LOGE("CONF_SERVER", "Listen task failed\r\n");
	}

	while (1)
	{
		if ((user_socket = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t*) &client_addr))<0)
		{
			// log and print the error
			ESP_LOGE("CONF_SERVER", "Failed to retrieve socket handler for incoming connection\r\n");
		}

		// create a task which communicates with client application to receive home network information and commands
		xTaskCreate(conf_server, "conf_server", 1024, NULL, 5, NULL);

		// wait until some event happens

		// check if STA disconnected or client app closed connection - wait for a new connection if configuration was not finished

		// check if all configuration information received and have communication with MQTT broker

	}



//	ESP_LOGI("CONF_SERVER", "Connection received\r\n");
//
//	if(recv(user_socket, message, 200, 0) < 0)
//	{
//		ESP_LOGE("CONF_SERVER", "Failed to receive a message\r\n");
//	}
//	strcpy(home_ssid,message);
//
//	ESP_LOGI("CONF_SERVER", "SSID received: %s\r\n",home_ssid);
//
//	if(recv(user_socket, message, 200, 0) < 0)
//	{
//		ESP_LOGE("CONF_SERVER", "Failed to receive a message\r\n");
//	}
//	strcpy(home_passwd,message);
//
//	ESP_LOGI("CONF_SERVER", "Password received: %s\r\n",home_passwd);
}


