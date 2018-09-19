#ifndef Adc_h
#define Adc_h

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <esp_log.h>

void adc_init(void* args);
uint32_t adc_read(int channel, bool dolog);
uint32_t filterADC(int channel, uint32_t vol);

#endif