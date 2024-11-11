#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "http_message.h"

#define DEFAULT_PORT 80
#define BUFFER_SIZE 1024

int check(int exp, const char *msg) {
    if (exp < 0) {
        perror(msg);
        exit(1);
    }
    return exp;
}
int respond_to_http_client_message(int sock_fd, http_client_message_t* http_msg)
{
    char *response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
    write(sock_fd, response, strlen(response));
    return 0;
}

void* handleConnection(void* a_client_ptr)
{
    int a_client = *(int*)a_client_ptr;
    free(a_client_ptr);

    while(1) {
        char buffer[BUFFER_SIZE];
        http_client_message_t* http_msg;
        http_read_result_t result;

        read_http_client_message(a_client, &http_msg, &result);
        if (result == BAD_REQUEST) { 
            printf("Connection closed\n");
            close(a_client);
            exit(1); 
        } else if (result == CLOSED_CONNECTION) {
            printf("Connection closed\n");
            close(a_client);
            exit(1);
        }

        respond_to_http_client_message(a_client, http_msg);
        http_client_message_free(http_msg);
    }
    printf("Finished connection %d\n", a_client);
}

int main(int argc, char const *argv[]) {    
    if (argc != 3 || strcmp(argv[1], "-p") != 0) {
        printf("Usage: <./filename> -p <port>\n");
        exit(1);
    }

    int socket_fd, client_sock;
    struct sockaddr_in sock_addr, client_addr;
    socklen_t client_addr_size;

    check(socket_fd = socket(AF_INET, SOCK_STREAM, 0), "Failed to create a socket");

    int port = atoi(argv[2]);

    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr.sin_port = htons(port);

    int returnval;

    check(returnval = bind(socket_fd,(struct sockaddr*)&sock_addr, sizeof(sock_addr))
                      , "Failed to bind");

    check(returnval = listen(socket_fd, 5), "Failed to listen");

    printf("Server listening on port %d. Waiting for a connection...\n", port);
    while (1) {
        socklen_t client_addr_len = sizeof(client_addr);
    
        check(client_sock = accept(socket_fd, (struct sockaddr *)&sock_addr, (socklen_t*)&client_addr_size)
                            , "Failed to accept connection");


        printf("Connected to client\n");
        int* client_sock_ptr = (int*)malloc(sizeof(int));
        *client_sock_ptr = client_sock;

        pthread_t thread;
        pthread_create(&thread, NULL, handleConnection, (void*)client_sock_ptr);
    
    }
    return 0;
}