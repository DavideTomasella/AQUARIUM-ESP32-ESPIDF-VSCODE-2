#ifndef Ultrasound_h
#define Ultrasound_h

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <esp_log.h>

void ultra_init(void* args);
uint16_t ultra_trig(bool firstTime);
void echo_handle(void* args);

#endif