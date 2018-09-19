#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <esp_log.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "DigIO.h"
#include "Globals.h"


static char tag[]="digio";


//initialize dig
void dig_init(void* args){
    ESP_LOGI(tag, "DIGIO Initialing");

    ESP_ERROR_CHECK(gpio_set_direction(DIN1,GPIO_MODE_INPUT) );
    ESP_ERROR_CHECK(gpio_set_direction(DIN2,GPIO_MODE_INPUT) );
    ESP_ERROR_CHECK(gpio_set_direction(DIN3,GPIO_MODE_INPUT) );
    ESP_ERROR_CHECK(gpio_set_direction(DIN4,GPIO_MODE_INPUT) );
    ESP_LOGI(tag, "DIGINPUT initialized");

    //TODO MODE_OUTPUT_OD PER USCITE OPEN DRAIN
    //201/02/04 cambiato GPIO_MODE in GPIO_MODE_INPUT_OUTPUT per consentire
    //la lettura dei pin d'uscita
    ESP_ERROR_CHECK(gpio_set_direction(DOUT1,GPIO_MODE_INPUT_OUTPUT) );
    ESP_ERROR_CHECK(gpio_set_direction(DOUT2,GPIO_MODE_INPUT_OUTPUT) );
    ESP_ERROR_CHECK(gpio_set_direction(DOUT3,GPIO_MODE_INPUT_OUTPUT) );
    ESP_ERROR_CHECK(gpio_set_direction(DOUT4,GPIO_MODE_INPUT_OUTPUT) );
    ESP_ERROR_CHECK(gpio_set_direction(DOUT5,GPIO_MODE_INPUT_OUTPUT) );
    ESP_ERROR_CHECK(gpio_set_direction(DOUT6,GPIO_MODE_INPUT_OUTPUT) );
    ESP_ERROR_CHECK(gpio_set_direction(DOUT7,GPIO_MODE_INPUT_OUTPUT) );
    ESP_ERROR_CHECK(gpio_set_direction(DOUT8,GPIO_MODE_INPUT_OUTPUT) );

    ESP_LOGI(tag, "DIGOUTPUT initialized");
}

//read channel value
uint32_t dig_in_read(int channel){
uint32_t vol = 0;
    switch(channel){
        case 0: //all
            vol = dig_readallin();
            ESP_LOGI(tag,"DIN read as %i", vol);
            break;
        case 1:
            vol = gpio_get_level(DIN1);
            ESP_LOGI(tag,"DIN1 read as %i", vol);
            break;
        case 2:
            vol = gpio_get_level(DIN2);
            ESP_LOGI(tag,"DIN2 read as %i", vol);
            break;
        case 3:
            vol = gpio_get_level(DIN3);
            ESP_LOGI(tag,"DIN3 read as %i", vol);
            break;
        case 4:
            vol = gpio_get_level(DIN4);
            ESP_LOGI(tag,"DIN4 read as %i", vol);
            break;
    }
    return vol;
}

//read channel value
uint32_t dig_out_read(int channel){
    uint32_t vol = 0;
    switch(channel){
        case 0: //all
            vol = dig_readallout();
            ESP_LOGI(tag,"DOUT read as %i", vol);
            break;
        case 1:
            vol = gpio_get_level(DOUT1);
            ESP_LOGI(tag,"DOUT1 read as %i", vol);
            break;
        case 2:
            vol = gpio_get_level(DOUT2);
            ESP_LOGI(tag,"DOUT2 read as %i", vol);
            break;
        case 3:
            vol = gpio_get_level(DOUT3);
            ESP_LOGI(tag,"DOUT3 read as %i", vol);
            break;
        case 4:
            vol = gpio_get_level(DOUT4);
            ESP_LOGI(tag,"DOUT4 read as %i", vol);
            break;
        case 5:
            vol = gpio_get_level(DOUT5);
            ESP_LOGI(tag,"DOUT5 read as %i", vol);
            break;
        case 6:
            vol = gpio_get_level(DOUT6);
            ESP_LOGI(tag,"DOUT6 read as %i", vol);
            break;
    }
    return vol;
}

//set channel value
void dig_out_set(int channel, uint32_t value){
    switch(channel){
        case 0: //all
            dig_setallout(value);
            ESP_LOGI(tag,"DOUT set as %i", value);
            break;
        case 1:
            ESP_ERROR_CHECK(gpio_set_level(DOUT1, value));
            ESP_LOGI(tag,"DOUT1 set as %i", value);
            break;
        case 2:
            ESP_ERROR_CHECK(gpio_set_level(DOUT2, value));
            ESP_LOGI(tag,"DOUT2 set as %i", value);
            break;
        case 3:
            ESP_ERROR_CHECK(gpio_set_level(DOUT3, value));
            ESP_LOGI(tag,"DOUT3 set as %i", value);
            break;
        case 4:
            ESP_ERROR_CHECK(gpio_set_level(DOUT4, value));
            ESP_LOGI(tag,"DOUT4 set as %i", value);
            break;
        case 5:
            ESP_ERROR_CHECK(gpio_set_level(DOUT5, value));
            ESP_LOGI(tag,"DOUT5 set as %i", value);
            break;
        case 6:
            ESP_ERROR_CHECK(gpio_set_level(DOUT6, value));
            ESP_LOGI(tag,"DOUT6 set as %i", value);
            break;
        case 7:
            ESP_ERROR_CHECK(gpio_set_level(DOUT7, value));
            ESP_LOGI(tag,"DOUT7 set as %i", value);
            break;
        case 8:
            ESP_ERROR_CHECK(gpio_set_level(DOUT8, value));
            ESP_LOGI(tag,"DOUT8 set as %i", value);
            break;
    }
}

uint32_t dig_readallin(){
    uint32_t vol = 0x00;
    if(gpio_get_level(DIN1) == 1)
        vol|=0x01;
    if(gpio_get_level(DIN2) == 1)
        vol|=0x02;
    if(gpio_get_level(DIN3) == 1)
        vol|=0x04;
    if(gpio_get_level(DIN4) == 1)
        vol|=0x08;
    return vol;
}

uint32_t dig_readallout(){
    uint32_t vol = 0x00;
    if(gpio_get_level(DOUT1) == 1)
        vol|=0x01;
    if(gpio_get_level(DOUT2) == 1)
        vol|=0x02;
    if(gpio_get_level(DOUT3) == 1)
        vol|=0x04;
    if(gpio_get_level(DOUT4) == 1)
        vol|=0x08;
    if(gpio_get_level(DOUT5) == 1)
        vol|=0x10;
    if(gpio_get_level(DOUT6) == 1)
        vol|=0x20;
    if(gpio_get_level(DOUT7) == 1)
        vol|=0x40;
    if(gpio_get_level(DOUT8) == 1)
        vol|=0x80;
    return vol;
}

void dig_setallout(uint32_t value){
    uint32_t vol = 0;
    vol = (value&0x01);
        ESP_ERROR_CHECK(gpio_set_level(DOUT1, vol));
    vol = (value&0x02)>>1;
        ESP_ERROR_CHECK(gpio_set_level(DOUT2, vol));    
    vol = (value&0x04)>>2;
        ESP_ERROR_CHECK(gpio_set_level(DOUT3, vol));   
    vol = (value&0x08)>>3;
        ESP_ERROR_CHECK(gpio_set_level(DOUT4, vol));    
    vol = (value&0x10)>>4;
        ESP_ERROR_CHECK(gpio_set_level(DOUT5, vol));
    vol = (value&0x20)>>5;
        ESP_ERROR_CHECK(gpio_set_level(DOUT6, vol));
    vol = (value&0x40)>>6;
        ESP_ERROR_CHECK(gpio_set_level(DOUT7, vol));
    vol = (value&0x80)>>7;
        ESP_ERROR_CHECK(gpio_set_level(DOUT8, vol));
}
