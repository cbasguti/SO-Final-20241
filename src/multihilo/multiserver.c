#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "request.h"
#include "io_helper.h"
#include <bits/getopt_core.h>

char default_root[] = ".";

// Estructura para pasar los parámetros a los hilos
typedef struct {
    int conn_fd;
    char root_dir[512];
} thread_args_t;

// Función que maneja cada solicitud del cliente
void* handle_request(void* args) {
    thread_args_t* thread_args = (thread_args_t*) args;
    int conn_fd = thread_args->conn_fd;
    char* root_dir = thread_args->root_dir;

    // Leer la solicitud HTTP
    char request[1024];
    read(conn_fd, request, sizeof(request) - 1);

    // Extraer el archivo solicitado
    char file_path[512];
    if (strncmp(request, "GET / ", 6) == 0) {
        strcpy(file_path, "index.html");  // Asignar index.html si la solicitud es para la raíz
    } else {
        sscanf(request, "GET /%s ", file_path);  // Extraer el archivo solicitado
    }


    // Determinar el tipo de contenido
    const char *content_type = "text/html";
    if (strstr(file_path, ".css")) {
        content_type = "text/css";
    }

    // Construir la ruta completa del archivo
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", root_dir, file_path);

    // Abrir el archivo solicitado
    FILE *file = fopen(full_path, "r");
    if (file == NULL) {
        // Enviar respuesta 404 si el archivo no existe
        char response[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found";
        write(conn_fd, response, strlen(response));
    } else {
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
    }

    close_or_die(conn_fd);
    free(args);  // Liberar la memoria asignada para los argumentos del hilo
    pthread_exit(NULL);  // Terminar el hilo
}

int main(int argc, char *argv[]) {
    int c;
    char *root_dir = default_root;
    int port = 15000;
    
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

    // Cambiar el directorio base
    chdir_or_die(root_dir);

    // Iniciar el socket de escucha
    int listen_fd = open_listen_fd_or_die(port);

    while (1) {
        struct sockaddr_in client_addr;
        int client_len = sizeof(client_addr);
        int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);

        // Asignar memoria para los argumentos del hilo
        thread_args_t *args = malloc(sizeof(thread_args_t));
        args->conn_fd = conn_fd;
        strncpy(args->root_dir, root_dir, sizeof(args->root_dir) - 1);

        // Crear un nuevo hilo para manejar la solicitud
        pthread_t thread;
        pthread_create(&thread, NULL, handle_request, (void*) args);

        // Desvincular el hilo para que pueda limpiarse automáticamente cuando termine
        pthread_detach(thread);
    }

    return 0;
}
