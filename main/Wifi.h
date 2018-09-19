#ifndef Wifi_h
#define Wifi_h

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <esp_log.h>

#include "esp_event.h"

#include "ping/ping.h"
#include "ping/esp_ping.h"

esp_err_t event_handler(void *ctx, system_event_t *event);

void nvs_init();
void wifiudp_init(void *pvParameter);
void wifi_initial();
void wifi_terminate();
void wifi_init_default();
void wifi_config_choser();

esp_err_t pingResults(ping_target_id_t msgType, esp_ping_found * pf);
void ping_gateway(void *pvParameter);

#endif
