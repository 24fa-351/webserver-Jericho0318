#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
// #include <arpa/inet.h>
// #include <sys/stat.h>
// #include <fcntl.h>
#include "request.h"


bool read_request_line(Request* req, int fd);
bool read_headers(Request* req, int fd);
bool read_body(Request* req, int fd);

Request* request_read_from_fd(int fd) {
    printf("Reading request from fd %d\n", fd);

    Request* req = malloc(sizeof(Request));

    if (read_request_line(req,fd) == false) {
        printf("Failed to read request line\n");
        request_free(req);
        return NULL;
    }

    if (read_headers(req, fd) == false) {
        printf("Failed to read headers\n");
        request_free(req);
        return NULL;
    }

    if (read_body(req, fd) == false) {
        printf("Failed to read body\n");
        request_free(req);
        return NULL;
    }
    return req;
}

void request_print(Request* req) {
    printf("vvv Request vvv\n");
    if (req->method) {
        printf("Method: %s\n", req->method);
    }

    if (req->path) {
        printf("Path: %s\n", req->path);
    }

    if (req->version) {
        printf("Version: %s\n", req->version);
    }

    for (int ix = 0; ix < req->header_count; ix++) {
        printf("Header %d: %s: %s\n", ix, req->headers[ix].key,
                req->headers[ix].value);
    }

    printf("^^^ Request ^^^\n");
}

#define FREE_IF_NOT_NULL(ptr) if (ptr) { free(ptr); }
void request_free(Request* req) {
    printf("freeing request\n");
    if(req == NULL) {
        return;
    }
    FREE_IF_NOT_NULL(req->method);
    FREE_IF_NOT_NULL(req->path);
    FREE_IF_NOT_NULL(req->version);

    for (int ix = 0; ix < req->header_count; ix++) {
        FREE_IF_NOT_NULL(req->headers[ix].key);
        FREE_IF_NOT_NULL(req->headers[ix].value);
        
    }
    FREE_IF_NOT_NULL(req->headers);
    
    free(req);
}

char* read_line(int fd) {
    printf("Reading line from fd %d\n", fd);
    char* line = malloc(10000);
    int len_read = 0;
    while (1) {
        char ch;
        int number_bytes_read;
        number_bytes_read = read(fd, &ch, 1);

        if (number_bytes_read <= 0) {
            return NULL;
        }
        if (ch == '\n') {
            break;
        }
        line[len_read] = ch;
        len_read++;
        line[len_read] = '\0';
    }
    if (len_read > 0 && line[len_read - 1] == '\r') {
        line[len_read - 1] = '\0';
    }
    line = realloc(line, len_read + 1);
    return line;
}

bool read_request_line(Request* req, int fd) {
    printf("Reading request line\n");
    char* line = read_line(fd);
    if (line == NULL) {
        return false;
    }

    req->method = malloc(strlen(line) + 1);
    req->path = malloc(strlen(line) + 1);
    req->version = malloc(strlen(line) + 1);
    int length_parsed;
    int number_parsed;

    number_parsed = sscanf(line, "%s %s %s%n", req->method, req->path,
                           req->version, &length_parsed);
    if (number_parsed != 3 || length_parsed != strlen(line)) {
        printf("Failed to parse request line\n");
        free(line);
        return false;
    }

    if (strcmp(req->method, "GET") != 0 && strcmp(req->method, "POST") != 0) {
        printf("Invalid method: %s\n", req->method);
        free(line);
        return false;
    }

    // if (strncmp(req->path, "/static/",8) == 0) {
    //     printf("Static\n");
    //     server_static(fd, req->path + 11);
    // } else if (strcmp(req->path, "/stats") == 0) {
    //     printf("Invalid path: %s\n", req->path);
    //     free(line);
    // } else if (strncmp(req->path, "/calc",6) == 0) {
    //     printf("Invalid path: %s\n", req->path);
    //     free(line);
    // }
    return true;
}

bool read_headers(Request* req, int fd) {
    printf("Reading headers\n");
    req->headers = malloc(sizeof(Header) * 100);
    req->header_count = 0;
    while (1) {
        char* line = read_line(fd);
        if(line == NULL) {
            return false;
        }
        if(strlen(line) == 0) {
            free(line);
            break;
        }
        
        req->headers[req->header_count].key = malloc(10000);
        req->headers[req->header_count].value = malloc(10000);
        int number_parsed;
        int length_parsed;
        number_parsed = sscanf(line, "%s: %s%n", req->headers[req->header_count].key, 
                               req->headers[req->header_count].value, &length_parsed);
        if (number_parsed != 2 || length_parsed != strlen(line)) {
            printf("Failed to parse header\n");
            free(line);
            return false;
        }
        req->headers[req->header_count].key = realloc(req->headers[req->header_count].key,
                                              strlen(req->headers[req->header_count].key) + 1);
        req->headers[req->header_count].key = realloc(req->headers[req->header_count].value,
                                              strlen(req->headers[req->header_count].value) + 1);
        req->header_count++;
        free(line);
    }
    return true;

}
bool read_body(Request* req, int fd) {
    printf("Reading body\n");
    return true;
}