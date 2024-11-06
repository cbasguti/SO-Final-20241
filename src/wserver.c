#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "request.h"
#include "io_helper.h"

char default_root[] = ".";

//
// ./wserver [-d <basedir>] [-p <portnum>] 
// 
int main(int argc, char *argv[]) {
    int c;
    char *root_dir = default_root;
    int port = 10000;
    
    while ((c = getopt(argc, argv, "d:p:")) != -1)
	switch (c) {
	case 'd':
	    root_dir = optarg;
	    break;
	case 'p':
	    port = atoi(optarg);
	    break;
	default:
	    fprintf(stderr, "usage: wserver [-d basedir] [-p port]\n");
	    exit(1);
	}

    // run out of this directory
    chdir_or_die(root_dir);

    // now, get to work
    int listen_fd = open_listen_fd_or_die(port);
    while (1) {
        struct sockaddr_in client_addr;
        int client_len = sizeof(client_addr);
        int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);

        // Leer la solicitud HTTP
        char request[1024];
        read(conn_fd, request, sizeof(request) - 1);

        // Extraer el archivo solicitado
        char file_path[512];
        sscanf(request, "GET /%s ", file_path);

        // Si no se especifica un archivo, servir index.html
        if (strcmp(file_path, "") == 0) {
            strcpy(file_path, "index.html");
        }

        // Determinar el tipo de contenido
        const char *content_type = "text/html";
        if (strstr(file_path, ".css")) {
            content_type = "text/css";
        }

        // Abrir el archivo solicitado
        FILE *file = fopen(file_path, "r");
        if (file == NULL) {
            // Enviar respuesta 404 si el archivo no existe
            char response[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found";
            write(conn_fd, response, strlen(response));
            close_or_die(conn_fd);
            continue;
        }

        // Enviar encabezado de respuesta
        char response[1024];
        snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", content_type);
        write(conn_fd, response, strlen(response));

        // Enviar el contenido del archivo
        char buffer[1024];
        size_t n;
        while ((n = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            write(conn_fd, buffer, n);
        }

        fclose(file);
        close_or_die(conn_fd);
    }
    return 0;
}


    


 
