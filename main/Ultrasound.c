
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <esp_log.h>
#include <driver/gpio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "Ultrasound.h"
#include "Globals.h"

uint32_t endTime;
uint32_t startTime;
uint16_t Prev_distance = 0;
uint16_t distance = 0;
bool enable1 = false;
SemaphoreHandle_t xEcho;

static char tag[]="ultrasound";

// Similar to uint32_t system_get_time(void)
static uint32_t get_usec() {

    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (tv.tv_sec*1000000 + tv.tv_usec);

}


void gpio_isr_handle(void* args){ //IRAM_ATTR
    if(enable1&&gpio_get_level(ECHO_PIN)==1){
    startTime=get_usec();
    }
    if(enable1&&gpio_get_level(ECHO_PIN)==0){
    endTime=get_usec();
    xSemaphoreGiveFromISR(xEcho,NULL);
    }    
} 

void ultra_init(void* args){

    ESP_LOGI(tag, "PINS initializing");

    gpio_pad_select_gpio(TRIG_PIN);
    gpio_pad_select_gpio(ECHO_PIN);

    gpio_set_direction(TRIG_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);

    ESP_LOGI(tag, "INTERRUPT initializing");

    ESP_ERROR_CHECK(gpio_set_intr_type(ECHO_PIN,GPIO_INTR_ANYEDGE));
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_IRAM));
    ESP_ERROR_CHECK(gpio_isr_handler_add(ECHO_PIN,gpio_isr_handle,(void*)ECHO_PIN));

    xEcho = xSemaphoreCreateBinary();
    
    ESP_LOGI(tag, "ULTRASOUND initialized");

    ultra_trig(true);

}

uint16_t ultra_trig(bool firstTime){
    endTime = 0;
    startTime = 0;
    enable1=true;
    gpio_set_level(TRIG_PIN, 1);
    ets_delay_us(10);
    gpio_set_level(TRIG_PIN, 0);
    if(xSemaphoreTake(xEcho,10)==pdTRUE){
    }
    enable1=false;
    uint16_t vol = 0;
    if(endTime==0||startTime==0){
        ESP_LOGE(tag, "Not Respone Correctly -> MEASURE NOT SAVED");
    }
    else{
        int diff = endTime - startTime; // Diff time in uSecs
        vol = 340.29 * diff / (1000.0 * 2); // Distance in meters  
        if(firstTime) distance = vol;
        else distance = (31 * Prev_distance + 1 * vol)>>5;
        Prev_distance = distance;      
        //ESP_LOGI(tag, "time %i distance %i", diff, distance);
    }

    ESP_LOGI(tag, "read as %i %i", vol, distance);
    return Prev_distance;
}
