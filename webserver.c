#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include "request.h"

#define DEFAULT_PORT 8080

int total_requests = 0;
long total_bytes_received = 0;
long total_bytes_sent = 0;


int check(int exp, const char *msg) {
    if (exp < 0) {
        perror(msg);
        exit(1);
    }
    return exp;
}

void* handle_connection(void* sock_fd_ptr) {
    int sock_fd = *(int*)sock_fd_ptr;
    free(sock_fd_ptr);

    while (1) {
        Request* req = request_read_from_fd(sock_fd);
        if (req == NULL) {
            break;
        }

        total_requests++;

        if (strncmp(req->path, "/static/", 9) == 0) {
            server_static(sock_fd, req->path + 10);
        } else if (strcmp(req->path, "/stats") == 0) {
            char stats_response[1024];
            snprintf(stats_response, sizeof(stats_response),
                     "HTTP/1.1 200 OK\nContent-Type: text/html\n\n"
                     "Server Stats\n"
                     "Total Requests: %d\n"
                     "Total Bytes Received: %ld\n"
                     "Total Bytes Sent: %ld\n",
                     total_requests, total_bytes_received, total_bytes_sent);
            send(sock_fd, stats_response, strlen(stats_response), 0);
        } else if (strncmp(req->path, "/calc", 6) == 0) {
            int a = 0, b = 0;
            sscanf(strstr(req->path, "a=") + 2, "%d", &a);
            sscanf(strstr(req->path, "b=") + 2, "%d", &b);
            char calc_response[1024];
            snprintf(calc_response, sizeof(calc_response),
                     "HTTP/1.1 200 OK\nContent-Type: text/plain\n\n"
                     "Result: %d", a + b);
            send(sock_fd, calc_response, strlen(calc_response), 0);
        } else {
            const char* not_found = "HTTP/1.1 404 Not Found\n\n";
            send(sock_fd, not_found, strlen(not_found), 0);
        }

        request_free(req);
    }

    close(sock_fd);
    return NULL;
}

int main(int argc, char const *argv[]) {
    int port;
    if (argc == 3 && strcmp(argv[1], "-p") == 0) {
        port = atoi(argv[2]);
    } else if (argc == 2 && strcmp(argv[1], "-p") == 0) {
        port = DEFAULT_PORT;
    }

    int socket_fd, client_sock;
    struct sockaddr_in sock_addr, client_addr;
    socklen_t client_addr_size;

    check(socket_fd = socket(AF_INET, SOCK_STREAM, 0), "Failed to create socket");

    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr.sin_port = htons(port);

    check(bind(socket_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr)), "Failed to bind");
    check(listen(socket_fd, 5), "Failed to listen");

    printf("Server listening on port %d\n", port);
    while (1) {
        client_addr_size = sizeof(client_addr);
        check(client_sock = accept(socket_fd, (struct sockaddr*)&client_addr, &client_addr_size), "Failed to accept");

        int* client_sock_ptr = malloc(sizeof(int));
        *client_sock_ptr = client_sock;
        pthread_t thread;
        pthread_create(&thread, NULL, handle_connection, client_sock_ptr);
    }

    return 0;
}
