#include "comun.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>
#include <assert.h>

#define TOT_TEMAS 256

struct Tema
{
    char name[TOT_TEMAS];
    int count_addr;
    struct sockaddr_in server_addr[TOT_TEMAS];
};
struct Temas
{
    struct Tema tema[TOT_TEMAS];
    int count_temas;
} temas;

void read_temas_from_file(char *temas_file)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(temas_file, "r");
    if (fp == NULL)
    {
        perror("Intermediario: fopen() ERROR");
        exit(EXIT_FAILURE);
    }

    while ((read = getline(&line, &len, fp)) != -1)
    {
        line[strcspn(line, "\n")] = 0;
        struct Tema tema1;
        strcpy(tema1.name, line);
        temas.tema[temas.count_temas] = tema1;
        temas.count_temas += 1;
        printf("Intermediario: Leido tema: %s!\n", line);
    }

    fclose(fp);
    if (line)
        free(line);
}

int get_tema_pos(char *tema)
{
    int j;
    for (j = 0; j < temas.count_temas; j++)
    {
        if (strcmp(temas.tema[j].name, tema)==0)
            return j;
    }
    return -1;
}

int is_eq_socket(struct sockaddr_in *s1, struct sockaddr_in *s2)
{
    if (s1->sin_family != s2->sin_family)
        return -1;
    if (s1->sin_port != s2->sin_port)
        return -1;
    if (s1->sin_addr.s_addr != s2->sin_addr.s_addr)
        return -1;
    return 0;
}

int get_suscrito_pos(int pos, struct sockaddr_in *cliente)
{
    struct Tema tema1 = temas.tema[pos];
    int j;
    for (j = 0; j < tema1.count_addr; j++)
    {
        if (is_eq_socket(&tema1.server_addr[j], cliente) >= 0)
        {
            return j; //Already suscribed
        }
    }
    return -1;
}

int suscribe(int tema_pos, struct sockaddr_in *cliente)
{
    struct Tema tema1 = temas.tema[tema_pos];
    if (tema1.count_addr >= TOT_TEMAS)
    { //No more available addresses for this tema
        return -1;
    }

    struct sockaddr_in s_in = tema1.server_addr[tema1.count_addr];
    s_in.sin_family = cliente->sin_family;
    s_in.sin_port = cliente->sin_port;
    s_in.sin_addr = cliente->sin_addr;

    tema1.server_addr[tema1.count_addr] = s_in;
    tema1.count_addr += 1;

    temas.tema[tema_pos] = tema1;

    return 0;
}

int unsuscribe(int tema_pos, int serveraddr_pos, struct sockaddr_in *cliente)
{
    struct Tema tema1 = temas.tema[tema_pos];

    //Shift the array
    int j;
    for (j = serveraddr_pos; j < (tema1.count_addr - 1); j++)
    {
        tema1.server_addr[j] = tema1.server_addr[j + 1];
    }

    tema1.count_addr -= 1;
    temas.tema[tema_pos] = tema1;

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Uso: %s puerto fichero_temas\n", argv[0]);
        return 1;
    }

    // Read file and load Temas struct
    read_temas_from_file(argv[2]);

    struct sockaddr_in server;
    struct sockaddr_in client;
    int sd1, cd1, moption = 1;

    sd1 = socket(AF_INET, SOCK_STREAM, 0);
    if (sd1 < 0)
    {
        perror("Intermediario: socket(): ERROR\n");
        exit(1);
    }
    fprintf(stdout, "Intermediario: Creacion del socket: OK\n");

    //Reuse port
    if (setsockopt(sd1, SOL_SOCKET, SO_REUSEADDR, &moption, sizeof(moption)) < 0)
    {
        perror("Intermediario: setsockopt() ERROR");
        exit(1);
    }

    //Localhost:port assigned
    bzero((char *)&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(argv[1]));

    if (bind(sd1, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Intermediario: bind(): ERROR\n");
        exit(1);
    }
    fprintf(stdout, "Intermediario: bind(): OK\n");

    //Accept()
    if (listen(sd1, 5) < 0)
    {
        perror("Intermediario: listen(): ERROR\n");
        exit(1);
    }
    fprintf(stdout, "Intermediario: listen(): OK\n");

    while (1)
    {
        int size1 = sizeof(client);
        char buffer[BUFF_LEN];
        char *ret;

        cd1 = accept(sd1, (struct sockaddr *)&client, (socklen_t *)&size1);
        if (cd1 < 0)
        {
            perror("Intermediario: accept(): ERROR\n");
            exit(1);
        }
        fprintf(stdout, "Intermediario: accept(): OK\n");

        if (read(cd1, buffer, BUFF_LEN) < 0)
        {
            perror("Intermediario: read(): ERROR\n");
            exit(1);
        }
        fprintf(stdout, "Intermediario: read(): OK\n");

        // ALTA
        if ((ret = strstr(buffer, MSG_ALTA)) != NULL)
        {
            char tema[TOT_TEMAS];
            int port, pos;
            sscanf(ret + strlen(MSG_ALTA), "%s%d", tema, &port);
            client.sin_port = port;

            if ((pos = get_tema_pos(tema)) < 0)
            {
                printf("Intermediario: Alta: No existe el tema: ERROR\n");
                write(cd1, "ERROR", sizeof("ERROR"));
                continue;
            }
            if (get_suscrito_pos(pos, &client) >= 0)
            {
                printf("Intermediario: Alta: Cliente ya suscrito: ERROR\n");
                write(cd1, "ERROR", sizeof("ERROR"));
                continue;
            }
            if (suscribe(pos, &client) < 0)
            {
                printf("Intermediario: Alta: Demasiados suscriptores: ERROR\n");
                write(cd1, "ERROR", sizeof("ERROR"));
                continue;
            }
            printf("Intermediario: Alta: Cliente suscrito %s puerto %d: OK\n", temas.tema[pos].name, port);
            write(cd1, "OK", sizeof("OK"));
        }

        //BAJA
        else if ((ret = strstr(buffer, MSG_BAJA)) != NULL)
        {
            char tema[TOT_TEMAS];
            int port, pos, serveraddr_pos;
            sscanf(ret + strlen(MSG_BAJA), "%s%d", tema, &port);
            client.sin_port = port;

            if ((pos = get_tema_pos(tema)) < 0)
            {
                printf("Intermediario: BAJA: No existe el tema: ERROR\n");
                write(cd1, "ERROR", sizeof("ERROR"));
                continue;
            }
            if ((serveraddr_pos = get_suscrito_pos(pos, &client)) < 0)
            {
                printf("Intermediario: BAJA: Cliente no suscrito: ERROR\n");
                write(cd1, "ERROR", sizeof("ERROR"));
                continue;
            }
            if (unsuscribe(pos, serveraddr_pos, &client) < 0)
            {
                printf("Intermediario: BAJA: Demasiados suscriptores: ERROR\n");
                write(cd1, "ERROR", sizeof("ERROR"));
                continue;
            }
            printf("Intermediario: BAJA: Cliente suscrito: OK\n");
            write(cd1, "OK", sizeof("OK"));
        }

        //GENERAR
        else if ((ret = strstr(buffer, MSG_GEN)) != NULL)
        {
            char tema[TOT_TEMAS];
            int port, pos, sd2, j;
            sscanf(ret + strlen(MSG_GEN), "%s%d", tema, &port);
            client.sin_port = port;

            if ((pos = get_tema_pos(tema)) < 0)
            {
                printf("Intermediario: GEN: No existe el tema: ERROR\n");
                write(cd1, "ERROR", sizeof("ERROR"));
                continue;
            }

            struct Tema tema1 = temas.tema[pos];

            for (j = 0; j < tema1.count_addr; j++)
            { //Loop through all subscribed clients
                sd2 = socket(AF_INET, SOCK_STREAM, 0);
                if (sd2 < 0)
                {
                    perror("Intermediario: GEN: socket() ERROR");
                    continue;
                }
                fprintf(stdout, "Intermediario: GEN: socket(): OK\n");

                if (connect(sd2, (struct sockaddr *)&tema1.server_addr[j], sizeof(struct sockaddr_in)) < 0)
                {
                    perror("Intermediario: GEN: connect(): ERROR\n");
                    close(sd2);
                }
                fprintf(stdout, "Intermediario: GEN: connect(): OK\n");

                if (write(sd2, buffer, sizeof(buffer)) < 0)
                {
                    perror("Intermediario: GEN: write(): ERROR\n");
                    close(sd2);
                }
                fprintf(stdout, "Intermediario: GEN: connect(): OK\n");
            }
            if (j < (tema1.count_addr - 1))
            {
                write(cd1, "ERROR", sizeof("ERROR"));
            }
            write(cd1, "OK", sizeof("OK"));
        }
    }
	
    return 0;
}
