#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <esp_log.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "driver/ledc.h"

#include "Pwm.h"
#include "Globals.h"


static char tag[]="pwm";
ledc_channel_config_t ledc_channel1 = {};
ledc_channel_config_t ledc_channel2 = {};

//initialize pwm
void pwm_init(void* args){
    ESP_LOGI(tag, "PWM Initialing");
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .freq_hz = 5000,                      // frequency of PWM signal
        .speed_mode = LEDC_TIMER_0,           // timer mode
        .timer_num = LEDC_HIGH_SPEED_MODE     // timer index
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer) );
    ledc_channel1.channel    = AOUT1;
    ledc_channel1.duty       = 0;
    ledc_channel1.gpio_num   = AOUT1pin;
    ledc_channel1.speed_mode = LEDC_TIMER_0;
    ledc_channel1.timer_sel  = LEDC_HIGH_SPEED_MODE;
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel1) );
    ESP_LOGI(tag, "PWM1 initialized");
    ledc_channel2.channel    = AOUT2;
    ledc_channel2.duty       = 0;
    ledc_channel2.gpio_num   = AOUT2pin;
    ledc_channel2.speed_mode = LEDC_TIMER_0;
    ledc_channel2.timer_sel  = LEDC_HIGH_SPEED_MODE;
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel2) );
    ESP_LOGI(tag, "PWM2 initialized");
    //Automatic wake up and fall asleep
    ESP_ERROR_CHECK(ledc_fade_func_install(0) );

    //TODO TOGLIERE QUESTE FUNZIONI
    ledc_stop(ledc_channel1.speed_mode,ledc_channel1.channel,0);
    ledc_stop(ledc_channel2.speed_mode,ledc_channel2.channel,0);
}

void pwm_set(int channel, uint32_t duty){
    if(channel == 1){
        ESP_ERROR_CHECK(ledc_set_duty(ledc_channel1.speed_mode,
                        ledc_channel1.channel, 
                        duty) );
        ESP_ERROR_CHECK(ledc_update_duty(ledc_channel1.speed_mode, 
                        ledc_channel1.channel) );
    }
    else if(channel == 2){
        ESP_ERROR_CHECK(ledc_set_duty(ledc_channel2.speed_mode,
                        ledc_channel2.channel, 
                        duty) );
        ESP_ERROR_CHECK(ledc_update_duty(ledc_channel2.speed_mode, 
                        ledc_channel2.channel) );
    }

}

void pwm_goup(int channel, int fade_time){
    if(channel ==1){
        ESP_LOGI(tag, "PWM1 FADE UP STARTED %i",fade_time);
        ESP_ERROR_CHECK(ledc_set_fade_with_time(ledc_channel1.speed_mode,
                        ledc_channel1.channel, 
                        8191, 
                        fade_time) );
        ESP_ERROR_CHECK(ledc_fade_start(ledc_channel1.speed_mode,
                        ledc_channel1.channel, 
                        LEDC_FADE_NO_WAIT) );
    }
    else if(channel == 2){
        ESP_LOGI(tag, "PWM2 FADE UP STARTED");
        ESP_ERROR_CHECK(ledc_set_fade_with_time(ledc_channel2.speed_mode,
                        ledc_channel2.channel, 
                        8191, 
                        fade_time) );
        ESP_ERROR_CHECK(ledc_fade_start(ledc_channel2.speed_mode,
                        ledc_channel2.channel, 
                        LEDC_FADE_NO_WAIT) );
    }

}

void pwm_godown(int channel, int fade_time){
    if(channel == 1){
        ESP_LOGI(tag, "PWM1 FADE DOWN STARTED %i",fade_time);
        ESP_ERROR_CHECK(ledc_set_fade_with_time(ledc_channel1.speed_mode,
                        ledc_channel1.channel, 
                        0, 
                        fade_time) );
        ESP_ERROR_CHECK(ledc_fade_start(ledc_channel1.speed_mode,
                        ledc_channel1.channel, 
                        LEDC_FADE_NO_WAIT) );
    }
    else if(channel == 2){
        ESP_LOGI(tag, "PWM2 FADE DOWN STARTED");
        ESP_ERROR_CHECK(ledc_set_fade_with_time(ledc_channel2.speed_mode,
                        ledc_channel2.channel, 
                        0, 
                        fade_time) );
        ESP_ERROR_CHECK(ledc_fade_start(ledc_channel2.speed_mode,
                        ledc_channel2.channel, 
                        LEDC_FADE_NO_WAIT) );
    }
}

//read channel value
uint32_t pwm_read(int channel){
    uint32_t vol = 0;
    if(channel == 1){
        vol = ledc_get_duty(ledc_channel1.speed_mode,ledc_channel1.channel);
        ESP_LOGI(tag,"PWM1 read as %i", vol);
    }
    else if(channel == 2){
        vol = ledc_get_duty(ledc_channel2.speed_mode,ledc_channel2.channel);
        ESP_LOGI(tag,"PWM2 read as %i", vol);
    }
    return vol;
}