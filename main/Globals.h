
#ifndef Globals_h
#define Globals_h

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <esp_log.h>
#include "freertos/event_groups.h"

//variabili per udp
#define PORT_NUMBER 6789        //UDP Open Port
#define BUFLEN 512              //UDP buffer length

//variabili per timer
#define NUM_TIMER  6            //Number of Timer enabled

//variabili per main
#define BLINK_GPIO GPIO_NUM_27   //Blinking Led

//2018/02/02 variabili per adc
#define AIN1 ADC1_CHANNEL_0 //SENSOR_VP GPIO_36
#define AIN1pin (36)
#define AIN2 ADC1_CHANNEL_3 //SENSOR_VI GPIO_39
#define AIN2pin (39)

//2018/02/02 variabili per pwm
#define AOUT1 LEDC_CHANNEL_0
#define AOUT1pin (23)
#define AOUT2 LEDC_CHANNEL_1
#define AOUT2pin (22)

//2018/02/02 variabili per dig_io
#define DIN1 GPIO_NUM_34
#define DIN2 GPIO_NUM_35
#define DIN3 GPIO_NUM_32
#define DIN4 GPIO_NUM_33
#define DOUT1 GPIO_NUM_1
#define DOUT2 GPIO_NUM_3
#define DOUT3 GPIO_NUM_21
#define DOUT4 GPIO_NUM_19
#define DOUT5 GPIO_NUM_18
#define DOUT6 GPIO_NUM_5
#define DOUT7 GPIO_NUM_17
#define DOUT8 GPIO_NUM_16
#define ECHO_PIN GPIO_NUM_33
#define TRIG_PIN GPIO_NUM_27
/*
#ifdef __cplusplus
extern "C" {
#endif
    extern  gpio_num_t gpio_IN1;
    extern  gpio_num_t gpio_IN2;
#ifdef __cplusplus
}
#endif
*/


//2018/02/02 struttura per dati InOut
typedef struct
{
	uint16_t    InPins;
	uint16_t    Outpins;
	uint16_t    AdcCh1;
	uint16_t	AdcCh2;
    uint16_t    PwmCh1;
    uint16_t    PwmCh2;
    uint16_t    TrigEcho;
} INOUT_t;

//2018/02/03 struttura per i dati ricevuti
typedef struct
{
    uint16_t    Outpins;
    uint16_t    PwmState;
} recINOUT_t;

#ifdef __cplusplus
extern "C" {
#endif
    extern INOUT_t INOUTData;
    extern recINOUT_t recINOUTData;
    /* FreeRTOS event group to signal when we are connected & ready to make a request */
    extern EventGroupHandle_t wifi_event_group;
    #define CONNECTED_BIT BIT0
#ifdef __cplusplus
}
#endif

#endif /* Globals_h */
