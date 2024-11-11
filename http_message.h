#include <stdbool.h>

#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H

typedef struct msg {
    char *method;
    char *path;
    char *http_version;
    char *body;
    char *headers;
    int body_length;
} http_client_message_t;

typedef enum {
    BAD_REQUEST,
    CLOSED_CONNECTION,
    MESSAGE
} http_read_result_t;

bool is_complete_http_message(char* buffer);

void read_http_client_message(int client_sock,
    http_client_message_t **msg,
    http_read_result_t *result);

void http_client_message_free(http_client_message_t* msg);
#endif