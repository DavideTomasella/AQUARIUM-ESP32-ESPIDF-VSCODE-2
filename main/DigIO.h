#ifndef DigIO_h
#define DigIO_h

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <esp_log.h>

void dig_init(void* args);
uint32_t dig_in_read(int channel); //channel 0 for all
uint32_t dig_out_read(int channel); //channel 0 for all
void dig_out_set(int channel, uint32_t value); //channel 0 for all

uint32_t dig_readallin();
uint32_t dig_readallout();
void dig_setallout();

#endif