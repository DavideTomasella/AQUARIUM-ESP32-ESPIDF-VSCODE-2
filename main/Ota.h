#ifndef Ota_h
#define Ota_h

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <esp_log.h>

int ota_run_update(void* args);

#endif