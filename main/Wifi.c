#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <esp_log.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"

#include "driver/gpio.h"

#include "ping/ping.h"
#include "ping/esp_ping.h"

#include "Wifi.h"
#include "Globals.h"
#include "ErrorHandling.h"

static char tag[]="wifi_my";
int pingstate;

// Home Wifi Access Point
const char* ssid1 = "ITIS";
const char* password1 = "pocoposto!";//TODO
const char* ssid2 = "HOME";
const char* password2 = "ERMADADA4";//TODO
//switcher per gli ssid salvati
wifi_config_t sta_config1 = { };
wifi_config_t sta_config2 = { };
uint8_t wifi_choser = 1;

EventGroupHandle_t  wifi_event_group = NULL;

// Event Loop Handler
esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(tag, "start");
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGI(tag, "disconnect -> try to change ssid");
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            wifi_terminate();
            wifi_initial();
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            ESP_LOGI(tag, "connected");
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(tag, "event_handler:SYSTEM_EVENT_STA_GOT_IP!");
            ESP_LOGI(tag, "got ip:%s\n",
            ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            // IP is availiable, set connected_bit
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_AP_STACONNECTED:
            ESP_LOGI(tag, "station:" MACSTR " join,AID=%d\n",
                     MAC2STR(event->event_info.sta_connected.mac),
                     event->event_info.sta_connected.aid);
            break;
        case SYSTEM_EVENT_AP_STADISCONNECTED:
            ESP_LOGI(tag, "station:" MACSTR "leave,AID=%d\n",
                     MAC2STR(event->event_info.sta_disconnected.mac),
                     event->event_info.sta_disconnected.aid);
            break;
        default:
            break;
    }
    return ESP_OK;
}

//inizializzo wifi e server udp
void wifiudp_init(void *pvParameter)
{
    ESP_LOGI(tag,"connection inizializing");

    //initialize NVS
    nvs_init();    
    
    //Connect to the AP in STA mode
    tcpip_adapter_init();
	
    //Set Event Handler
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    
    //initialize wifi
    wifi_initial();
}

void nvs_init(){
    //DA CHIAMARE SEMPRE PRIMA WIFI
    //Risolto errore in esp_wifi_init
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    ESP_LOGI(tag,"NVS inizialized");
}

void wifi_initial(){
    strcpy((char*)sta_config1.sta.ssid, ssid1);
    strcpy((char*)sta_config1.sta.password, password1);
    sta_config1.sta.bssid_set = false;    
    strcpy((char*)sta_config2.sta.ssid, ssid2);
    strcpy((char*)sta_config2.sta.password, password2);
    sta_config2.sta.bssid_set = false;
    
    //load default
    wifi_init_default();

    //select mode
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    //modify config chosing the correct network
    wifi_config_choser();

    //start wifi module
    ESP_ERROR_CHECK(esp_wifi_start());

    //prepare ping
    pingstate=0;
}

void wifi_terminate(){
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_wifi_stop());
}

void wifi_init_default(){
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}

void wifi_config_choser(){
        
    if(wifi_choser == 1){
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config1));
        ESP_LOGI(tag, "connecting to WIFI  SSID:%s in UDP mode\n",ssid1);
        wifi_choser++;
    }
    else if(wifi_choser == 2){
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config2));
        ESP_LOGI(tag, "connecting to WIFI  SSID:%s in UDP mode\n",ssid2);
        wifi_choser=1;
    }
}


//PING
esp_err_t pingResults(ping_target_id_t msgType, esp_ping_found * pf){
   printf("AvgTime:%.1fmS Sent:%d Rec:%d Err:%d min(mS):%d max(mS):%d ", (float)pf->total_time/pf->recv_count, pf->send_count, pf->recv_count, pf->err_count, pf->min_time, pf->max_time );
   printf("Resp(mS):%d Timeouts:%d Total Time:%d\n",pf->resp_time, pf->timeout_count, pf->total_time);
   if(pf->timeout_count>0){
        pingstate++;
        if(pingstate>120){ //10 minutes
            esp_restart();
        }
   }
   else{
       pingstate=0;
   }
   return ESP_OK;
}

void ping_gateway(void *pvParameter){
    tcpip_adapter_ip_info_t ipInfo;
    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
    
    uint32_t gateway=ipInfo.gw.addr;
    uint32_t ping_count = 1;  //how many pings per report
    uint32_t ping_timeout = 4; //mS till we consider it timed out
    uint32_t ping_delay = 1; //mS between pings
    
    //configurazione
    esp_ping_set_target(PING_TARGET_IP_ADDRESS_COUNT, &ping_count, sizeof(uint32_t));
    esp_ping_set_target(PING_TARGET_RCV_TIMEO, &ping_timeout, sizeof(uint32_t));
    esp_ping_set_target(PING_TARGET_DELAY_TIME, &ping_delay, sizeof(uint32_t));
    esp_ping_set_target(PING_TARGET_IP_ADDRESS, &gateway, sizeof(uint32_t));
    esp_ping_set_target(PING_TARGET_RES_FN, &pingResults, sizeof(pingResults));
    
    //initializzazione
    ping_init();
}
