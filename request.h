#ifndef REQUEST_H
#define REQUEST_H

typedef struct {
    char* method;
    char* path;
    char* version;
} Request;

Request* request_read_from_fd(int fd);

void server_static(int client_sock, const char* filepath);

void request_free(Request* req);

#endif
