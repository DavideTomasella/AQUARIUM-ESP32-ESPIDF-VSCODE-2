
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <esp_log.h>
/*
 *NOTE: ADC2 is not avaible because WIFI module is enabled
 * 
 */
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "Adc.h"
#include "Globals.h"


static char tag[]="adc";

//FILTER MEMORY
uint64_t somma_ch1;
uint32_t VPrev_ch1;
uint64_t somma_ch2;
uint32_t VPrev_ch2;

//initialize adc1
void adc_init(void* args){
    ESP_LOGI(tag, "ADC1 Initialing");
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_12Bit));
    //0db->1.1V 2_5db->1.5V 6db->2.2V 11db->3.9V
    ESP_LOGI(tag, "Connect IN1 channel %i", AIN1);
    ESP_ERROR_CHECK(adc1_config_channel_atten(AIN1, ADC_ATTEN_6db));
    ESP_LOGI(tag, "Connect IN2 channel %i", AIN2);  
    ESP_ERROR_CHECK(adc1_config_channel_atten(AIN2, ADC_ATTEN_6db));
    ESP_LOGI(tag, "ADC1 initialized");

    VPrev_ch1 = adc1_get_raw(AIN1);
    VPrev_ch2 = adc1_get_raw(AIN2);
}

//read channel value
uint32_t adc_read(int channel, bool dolog){
    uint32_t vol = 0;
    uint32_t val = 0;
    if(channel == 1){
        val = adc1_get_raw(AIN1); 
        vol = filterADC(channel, val);
        if(dolog)ESP_LOGI(tag,"ADC1 read as %i %i", val, vol);
    }
    else if(channel == 2){
        val = adc1_get_raw(AIN2);
        vol = filterADC(channel, val);
        if(dolog)ESP_LOGI(tag,"ADC2 read as %i %i", val, vol);
    }

    return vol;
}

uint32_t filterADC(int channel, uint32_t vol){
    uint32_t res = 0;
    if(channel == 1){
        res = (127 * VPrev_ch1 + 1 * vol)>>7;
        VPrev_ch1 = res;
    }
    else if(channel == 2){
        res = (127 * VPrev_ch2 + 1 * vol)>>7;
        VPrev_ch2 = res;
    }
    /*if(channel==1){
        uint32_t delta = VPrev_ch1 - vol;
        somma_ch1 += delta;
        VPrev_ch1 = somma_ch1 >> 4;
        res = VPrev_ch1;
    }
    else if(channel==2){
        uint32_t delta = VPrev_ch2 - vol;
        somma_ch2 += delta;
        VPrev_ch2 = somma_ch2 >> 4;
        res = VPrev_ch2;
    }*/
    return res;
}