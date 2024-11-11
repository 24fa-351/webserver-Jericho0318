#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>

#include "http_message.h"

bool is_complete_http_message(char* buffer) {
    if (strlen(buffer) < 100) {
        if (strncmp(buffer, "GET",4) != 0) {
            if (strncmp(buffer, "/static/", 8) == 0) {
                return false;
            } else if (strncmp(buffer, "/stats", 7) == 0) {
                return false;
            } else if (strncmp(buffer, "/calc", 6) == 0) {
                return false;
            }
        }
    }
    if (strncmp(buffer + strlen(buffer) - 2, "\n\n", 2) != 0) {
        return false;
    }

    return false; 
}
void read_http_client_message(
    int client_sock,
    http_client_message_t **msg,
    http_read_result_t *result)
{
    *msg = malloc(sizeof(http_client_message_t));
    char buffer[1024];
    strcpy(buffer, "");

    if(!is_complete_http_message(buffer)){
        int bytes_read = read(client_sock, buffer+strlen(buffer)
                              , sizeof(buffer) - strlen(buffer));
        if (bytes_read == 0) {
            *result = CLOSED_CONNECTION;
            exit(1);
        }

        if (bytes_read < 0) {
            *result = BAD_REQUEST;
            exit(1);
        }
    }
    (*msg)->method = strdup(buffer + 4);
    *result = MESSAGE;
}

void http_client_message_free(http_client_message_t* msg) { free(msg); }