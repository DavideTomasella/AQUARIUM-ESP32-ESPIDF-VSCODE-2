#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <esp_log.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "Adc.h"
#include "Pwm.h"
#include "DigIO.h"
#include "Ultrasound.h"
#include "Globals.h"

static char tag[]="inout";
INOUT_t INOUTData = {};
recINOUT_t recINOUTData = {};

void UpdateINOUTData(bool dolog){

    INOUTData.AdcCh1 = adc_read(1, dolog);
    INOUTData.AdcCh2 = adc_read(2, dolog);
    INOUTData.InPins = dig_in_read(0);
    INOUTData.Outpins = dig_out_read(0);
    INOUTData.PwmCh1 = pwm_read(1);
    INOUTData.PwmCh2 = pwm_read(2);
    INOUTData.TrigEcho = ultra_trig(NULL);
}

void SetINOUTData(void *args){

    dig_out_set(0,recINOUTData.Outpins);
    
    if((recINOUTData.PwmState&0x01) == 0x01){
        if(pwm_read(1)==0){ //<8192
            pwm_goup(1,6000000); //15 minute
        }
    }
    if((recINOUTData.PwmState&0x01) == 0x00){
        if(pwm_read(1)==8191){ //>0
            pwm_godown(1,6000000); //15 minute
        }
    }
    if((recINOUTData.PwmState&0x02) == 0x02){
        if(pwm_read(2)==0){ //<8192
            pwm_goup(2,30000);
        }
    }
    if((recINOUTData.PwmState&0x02) == 0x00){
        if(pwm_read(2)==8191){ //>0
            pwm_godown(2,30000);
        }
    }

}