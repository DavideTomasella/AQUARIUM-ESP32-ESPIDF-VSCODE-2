#ifndef UDPServer_h
#define UDPServer_h

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <esp_log.h>

#include "esp_event.h"

esp_err_t udp_server(void* args);

#endif
