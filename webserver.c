#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "request.h"

#define DEFAULT_PORT 46645

int check(int exp, const char *msg) {
    if (exp < 0) {
        perror(msg);
        exit(1);
    }
    return exp;
}

void server_dispatch_request(Request* req, int fd) {
    printf("Dispatching request\n");
}

void* handleConnection(int* sock_fd_ptr)
{
    int sock_fd = *sock_fd_ptr;
    free(sock_fd_ptr);

    while(1) {
        Request* req = request_read_from_fd(sock_fd);
        if (req == NULL) {
            break;
        }
        request_print(req);

        server_dispatch_request(req, sock_fd);
        request_free(req);
    }
    printf("Finished connection %d\n", sock_fd);
    close(sock_fd);
}

int main(int argc, char const *argv[]) {    
    if (argc == 2 && !strcmp(argv[1], "--request") != 0) {
        printf("Reading ONE request from stdin\n");
        Request* req = request_read_from_fd(0);
        if (req == NULL) {
            printf("Failed to read request\n");
            exit(1);
        }
        request_print(req);
        request_free(req);
        exit(0);
    }

    if (argc == 2 && !strcmp(argv[1], "--handle") != 0) {
        printf("Reading ONE connection from stdin\n");
        int* sock_fd = malloc(sizeof(int));
        *sock_fd = 0;
        handleConnection(sock_fd);
        printf("Done handling connection\n");
        exit(0);
    }

    int socket_fd, client_sock;
    struct sockaddr_in sock_addr, client_addr;
    socklen_t client_addr_size;

    check(socket_fd = socket(AF_INET, SOCK_STREAM, 0), "Failed to create a socket");


    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr.sin_port = htons(DEFAULT_PORT);

    int returnval;

    check(returnval = bind(socket_fd,(struct sockaddr*)&sock_addr, sizeof(sock_addr))
                      , "Failed to bind");

    check(returnval = listen(socket_fd, 5), "Failed to listen");

    printf("Server listening on port %d. Waiting for a connection...\n", DEFAULT_PORT);
    while (1) {
        socklen_t client_addr_len = sizeof(client_addr);
    
        check(client_sock = accept(socket_fd, (struct sockaddr *)&sock_addr, (socklen_t*)&client_addr_size)
                            , "Failed to accept connection");


        printf("Connected to client\n");
        int* client_sock_ptr = (int*)malloc(sizeof(int));
        *client_sock_ptr = client_sock;

        pthread_t thread;
        pthread_create(&thread, NULL, (void* (*)(void*))handleConnection, client_sock_ptr);
    
    }
    return 0;
}