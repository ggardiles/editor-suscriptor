#include "subscriptor.h"
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
#include <stdint.h>

int event_port;
void (*notifica_evento)(const char *, const char *);
void (*alta_de_tema)(const char *);
void (*baja_de_tema)(const char *);

void *server_listen_thread(void *arg)
{

    struct sockaddr_in client;
    int client_size = sizeof(client);
    int sd1 = (intptr_t)arg;

    while (1)
    {
		char buff[BUFF_LEN];
		char tema[TOT_TEMAS];
		char valor[TOT_TEMAS];
		int nleidos, sd2;
		char *ret;

		sd2 = accept(sd1, (struct sockaddr *)&client, (socklen_t *)&client_size);
		if (sd2 < 0)
		{
			perror("server_listen_thread: accept(): ERROR\n");
			close(sd1);
			exit(1);
		}
		fprintf(stdout, "server_listen_thread: accept(): OK\n");

		if ((nleidos = read(sd2, buff, TOT_TEMAS)) < 0)
		{
			perror("server_listen_thread: read(): ERROR\n");
			close(sd1);
			exit(1);
		}
		fprintf(stdout, "server_listen_thread: read(): OK\n");

		//Tema Creado
		if ((ret = strstr(buff, MSG_CREAT)) != NULL)
		{
			sscanf(ret + strlen(MSG_CREAT), "%s", tema);
			printf("Suscriptor: Alta tema: %s\n", tema);
			alta_de_tema(tema);
		}

		// Tema Eliminado
		else if ((ret = strstr(buff, MSG_DEL)) != NULL)
		{
			sscanf(ret + strlen(MSG_DEL), " %s", tema);
			printf("Suscriptor: Baja tema: %s\n", tema);
			baja_de_tema(tema);
		}

		// Mensaje Generado en Tema
		else if ((ret = strstr(buff, MSG_GEN)) != NULL)
		{
			sscanf(ret + strlen(MSG_GEN), "%s %s", tema, valor);
			printf("Suscriptor: GENERADO tema %s valor %s\n", tema,valor);
			notifica_evento(tema, valor);
		}
    }
}

int alta_subscripcion_tema(const char *tema)
{
    fprintf(stdout, "Called alta_subscripcion_tema\n");
    char buff_msg[BUFF_LEN];

    sprintf(buff_msg, "%s %s %d", MSG_ALTA, tema, event_port);
    return pasar_mensaje(buff_msg);
}

int baja_subscripcion_tema(const char *tema)
{
    fprintf(stdout, "Called baja_subscripcion_tema");
    char buff_msg[BUFF_LEN];

    sprintf(buff_msg, "%s %s %d", MSG_BAJA, tema, event_port);
    return pasar_mensaje(buff_msg);
}

int inicio_subscriptor(void (*notif_evento)(const char *, const char *),
		       void (*alta_tema)(const char *),
		       void (*baja_tema)(const char *))
{

    struct sockaddr_in server;
    pthread_t thread;
    int sd1, val_opcion = 1;
    int port_size = sizeof(server);

	// Punteros a funcion
    notifica_evento = notif_evento;
    alta_de_tema = alta_tema;
    baja_de_tema = baja_tema;

    // Creacion del socket
    sd1 = socket(AF_INET, SOCK_STREAM, 0);
    if (sd1 < 0)
    {
	perror("Suscriptor-Server: Creacion del socket: ERROR\n");
	exit(1);
    }
    fprintf(stdout, "Suscriptor-Server: Creacion del socket: OK\n");

    //Reuse port immediately
    if (setsockopt(sd1, SOL_SOCKET, SO_REUSEADDR, &val_opcion, sizeof(val_opcion)) < 0)
    {
	perror("Suscriptor-Server: setsockopt() ERROR");
	exit(1);
    }

    bzero((char *)&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(0);

    if (bind(sd1, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
	perror("Suscriptor-Server: bind(): ERROR\n");
	close(sd1);
	exit(1);
    }
    fprintf(stdout, "Suscriptor-Server: bind(): OK\n");

    if (getsockname(sd1, (struct sockaddr *)&server, (socklen_t *)&port_size) < 0)
    {
	fprintf(stdout, "Suscriptor-Server: getsockname(): ERROR\n");
	close(sd1);
	exit(1);
    }
    fprintf(stdout, "Suscriptor-Server: getsockname(): OK\n");

    event_port = server.sin_port;

    if (listen(sd1, 5) < 0)
    {
	perror("Suscriptor-Server: listen(): ERROR\n");
	close(sd1);
	exit(1);
    }
    fprintf(stdout, "Suscriptor-Server: listen(): OK\n");

    pthread_create(&thread, NULL, server_listen_thread, (void *)(intptr_t)sd1);

    return 0;
}

int fin_subscriptor()
{
    fprintf(stdout, "Called fin_subscriptor\n");
    char buff_msg[BUFF_LEN];

    sprintf(buff_msg, "%s %d", MSG_END, event_port);
    return pasar_mensaje(buff_msg);
}
