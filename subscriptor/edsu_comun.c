/*
   Incluya en este fichero todas las implementaciones que pueden
   necesitar compartir los módulos editor y subscriptor,
   si es que las hubiera.
*/
#include "comun.h"
#include "edsu_comun.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>

int pasar_mensaje(char *msg_preparado){

    struct sockaddr_in server_addr;
	struct hostent *server_info;
	int port, sd1;
	char buff_msg[BUFF_LEN], buff[BUFF_LEN], *server;

	//Retrieve server and port
	server = getenv(SERVER);
	if( server == NULL){
		perror("edsu_comun: getenv() Server not specified: ERROR ");
		exit(1);
	}
	if(getenv(PORT) == NULL){
		perror("edsu_comun: getenv() PORT not specified: ERROR ");
		exit(1);
	}
	port=atoi(getenv(PORT));
	
	// Configure protocol, address and port
	server_addr.sin_family=AF_INET;
	server_info=gethostbyname(server);
	memcpy(&server_addr.sin_addr.s_addr, server_info->h_addr, server_info->h_length);
	server_addr.sin_port=htons(port);
 
 	// Socket creation
	if ((sd1 = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("edsu_comun: Socket() ERROR\n");
		exit(1);
    }
	fprintf(stdout,"edsu_comun: Socket() OK\n");

	//Open connection with Broker
	if ((connect(sd1, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0){
		perror("edsu_comun: Connect() ERROR\n");
		close(sd1);
		exit(1);
	}
	fprintf(stdout,"edsu_comun: Connect() OK\n");

	//Message generation
	if ((write(sd1, msg_preparado , sizeof(buff_msg))) < 0){
		perror("edsu_comun: Write() ERROR\n");
		close(sd1);
		exit(1);
	}
	fprintf(stdout,"edsu_comun: Write(%s) OK\n", msg_preparado);
	
	//Read Broker's answer'
	if ((read(sd1, buff, sizeof(buff))) < 0){
		perror("edsu_comun: Read() ERROR\n");
		close(sd1);
		exit(1);
	}
	fprintf(stdout,"edsu_comun: Read() OK\n");
	
	//Validate Broker's response
	if ((strcmp(buff,"OK")) != 0){ //si status es igual a 0, ambas cadenas tienen lo mismo, es decir, buff lleva un OK
		fprintf(stderr, "edsu_comun: Read value -> %s\n",buff);
		close(sd1);
		return -1;
	}

	fprintf(stdout,"edsu_comun: Read value -> %s\n",buff);	
	close(sd1);
	return 0;
}
