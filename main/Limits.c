#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <esp_log.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "Limits.h"
#include "Globals.h"
#include "DigIO.h"
#include "Pwm.h"

static char tag[] = "LIMITS";

void CheckLimits(){
    if(INOUTData.AdcCh1<100||INOUTData.AdcCh1>4000){
        //PH ERROR OCCURED
        if(INOUTData.PwmCh1>8000){            
            ESP_LOGE(tag,"ADC2-PH: Not Responde Correctly -> TURN OFF CO2");
            dig_out_set(5,0);
        }        
        else{
            ESP_LOGE(tag,"ADC2-PH: Not Responde Correctly -> TURN ON CO2");
            dig_out_set(5,1);
        }
        
    }
    if(INOUTData.AdcCh2<100||INOUTData.AdcCh2>4000){
        ESP_LOGE(tag,"ADC2-TEMPERATURE: Not Responde Correctly -> TURN ON FANS");
        dig_out_set(3,0);
    }
}