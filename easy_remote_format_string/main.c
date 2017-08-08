// gcc main.c -m32 -z execstack -fno-stack-protector -Wno-format-security -no-pie -Wl,-z,norelro
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8282
#define BUFFER_SIZE 512

#define die() { fprintf(stderr, "died at line %u\n", __LINE__); exit(__LINE__); }

extern char **environ;

void
my_echo(int32_t fd) {
    int32_t n = 0;
    char client_buffer[BUFFER_SIZE] = "";
    char server_buffer[BUFFER_SIZE] = "";

    while (1) {
        memset(client_buffer, 0, sizeof(client_buffer));
        memset(server_buffer, 0, sizeof(client_buffer));
        if ((n = read(fd, client_buffer, sizeof(client_buffer))) < 0)
            die();
        snprintf(server_buffer, sizeof(server_buffer), client_buffer);
        write(fd, server_buffer, sizeof(server_buffer));
    }
}

int
main(void) {
    int32_t server_socket_fd, client_length, client_socket_fd;
    int32_t pid;
    struct sockaddr_in server_addr, client_addr;

    for (int i = 0; environ[i] != NULL; i++)
        memset(environ[i], 0, sizeof(environ[i]));
    if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        die();
    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        die();
    listen(server_socket_fd, 5);
    client_length = sizeof(client_addr);
    while (1) {
        client_socket_fd = accept(server_socket_fd, (struct sockaddr *)&client_addr, &client_length);
        if (client_socket_fd < 0)
            die();
        if ((pid = fork()) < 0)
            die();
        if (0 == pid) {
            close(server_socket_fd);
            my_echo(client_socket_fd);
            exit(0);
        }
        else
            close(client_socket_fd);
    }
    return 0;
}