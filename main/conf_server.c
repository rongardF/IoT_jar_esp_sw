#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "conf_server.h"

// this creates a server which accepts connection from the user to configure the device
//void conf_server (){
//	struct sockaddr_in addr;
//	int32_t sockfd, user_socket;
//
//	if (( sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0){ // create a socket handler
//		// log and print the error
//	}
//
//	addr.sin_family = AF_INET;
//	addr.sin_addr.s_addr = inet_addr(CONF_IP_ADDR); // convert IP address string to long format and assign it into IP address variable in structure
//	addr.sin_port = htons(CONF_PORT_NUM);
//
//	if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0){
//		// log and print the error
//	}
//
//	if (listen(sockfd, 3) < 0){  // listen for incoming connections; up to three connections can be in a queue
//		// log and print the error
//	}
//
//	if ((user_socket = accept(sockfd, (struct sockaddr *)&addr, (socklen_t*) &addr))<0){
//		// log and print the error
//	}
//
//
//}


