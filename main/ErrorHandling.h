#ifndef ErrorHandling_h
#define ErrorHandling_h

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <esp_log.h>

int get_socket_error_code(int socket);
int show_socket_error_reason(int socket);
void close_socket(int socket);

#endif
