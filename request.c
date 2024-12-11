#include "request.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

Request* request_read_from_fd(int fd) {
    Request* req = malloc(sizeof(Request));
    req->method = malloc(1000);
    req->path = malloc(1000);
    req->version = malloc(1000);

    char buffer[BUFFER_SIZE];
    int bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0)
        return NULL;
    buffer[bytes_read] = '\0';

    sscanf(buffer, "%s %s %s", req->method, req->path, req->version);
    return req;
}

void request_free(Request* req) {
    if (req) {
        free(req->method);
        free(req->path);
        free(req->version);
        free(req);
    }
}

void server_static(int client_sock, const char* filepath) {
    char fullpath[BUFFER_SIZE];
    snprintf(fullpath, sizeof(fullpath), "static/%s", filepath);

    int file_fd = open(fullpath, O_RDONLY);
    if (file_fd < 0) {
        const char* not_found = "HTTP/1.1 404 Not Found\n\n";
        send(client_sock, not_found, strlen(not_found), 0);
        return;
    }

    struct stat st;
    fstat(file_fd, &st);
    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header), "HTTP/1.1 200 OK\nContent-Length: %ld\n\n",
             st.st_size);
    send(client_sock, header, strlen(header), 0);

    char file_buffer[BUFFER_SIZE];
    int bytes;
    while ((bytes = read(file_fd, file_buffer, sizeof(file_buffer))) > 0)
        send(client_sock, file_buffer, bytes, 0);

    close(file_fd);
}
