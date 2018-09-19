#ifndef Pwm_h
#define Pwm_h

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <esp_log.h>

void pwm_init(void* args);
void pwm_set(int channel, uint32_t duty);
void pwm_goup(int channel, int fade_time);
void pwm_godown(int channel, int fade_time);
uint32_t pwm_read(int channel);

#endif