#include "editor.h"
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

int generar_evento(const char *tema, const char *valor) {
	char buff_msg[BUFF_LEN];

	sprintf(buff_msg, "%s %s %s", MSG_GEN, tema, valor);
	return pasar_mensaje(buff_msg);
}

/* solo para la version avanzada */
int crear_tema(const char *tema) {
	char buff_msg[BUFF_LEN];

	sprintf(buff_msg, "%s %s", MSG_CREAT, tema);
	return pasar_mensaje(buff_msg);
}

/* solo para la version avanzada */
int eliminar_tema(const char *tema) {
	char buff_msg[BUFF_LEN];

	sprintf(buff_msg, "%s %s", MSG_DEL, tema);
	return pasar_mensaje(buff_msg);
}

