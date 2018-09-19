/*
 * AQUARIUM
 * Powered By Davide Tomasella
 * 
 * Copyrigth (c) 2018
 *
 * Davide Tomasella programmer
 * Latest version 3.1 on 18/09/2018
 * 
 * 
 * DESCRIPTION:
 * 
 * ...
 * ...
 * 
 * TODO:
 * Get Time from UDP
 * Check light sleep mode with WiFi wake up
 * 
 * 
 * HISTORY:
 * 
 * 
 * Version 3.1
 * Add Ota updater
 * WARNING: OTA WORKS WITH HTTP CLIENT
 *          NO CERTIFICATES ARE NEEDED
 * 
 * Version 3.0
 * Update esp-idf library
 * 
 * Version 2.3
 * Update WiFi init -> NOTE: NO PING LET TO RESTART
 * Adjust Filters
 * On start-up correct set pwm value
 * Correct log message
 * 
 * Version 2.2
 * Add timers to read ADC
 * Filters less selective
 * On start-up initialize filters without For-cycle (take too long)
 * 
 * Version 2.1
 * Filters more selective
 * For-cycle on start-up to make the values instantly correctly
 * 
 * Version 2.0
 * Add Limits
 * Control for impossible input
 * 
 * Version 1.10
 * Correct pwm
 * IMPORTANT: check no 100% duty-cycle
 * 
 * Version 1.9
 * Correct negative logic out pins
 * Add LP filter at ultrasound.c
 * Modify fade pwm time
 * 
 * Version 1.8
 * add FIR on ADC input TODO: decide filter selectility
 * 
 * Version 1.7
 * Update udp protocol (Version 01.5)
 * Correct adc attenuation to 6db to obtain a 2.2V vref
 * 
 * Version 1.6
 * Enable ultrasound sensor (TRIG and ECHO)
 * Enable interrupt on ECHO
 * Enable semaphore on ISR
 * Add Ultrasound
 * 
 * Version 1.5
 * Added udp protocol functions (Version 01.4)
 * Add Wifi (divided udp server and wifi connection)
 * 
 * Version 1.4
 * *Removed not used files
 * INOUTData sent with udp server
 * Udp Server Version 01.3
 * SetINOUTData and UpdateINOUTData called by udp command 2
 * Resolved DigIO problems
 * 
 * Version 1.3
 * Enabled Pwm
 * Enabled read and set gpio DigIO
 * Created INOUTData struct
 * Read InOut
 * Set InOut
 * Config Fade Pwm
 * Add Pwm
 * Add InOut
 * Add DigIO
 * 
 * Version 1.2
 * Enabled ADC
 * Add ADC
 * Multiple wifi handling
 * 
 * Version 1.1
 * Enabled Watchdog on CPU1
 * Enabled timer and task handling
 * Udp Server Version 01.2
 * Add Timer
 * 
 * Version 1.0
 * Enabled COM comunication
 * Enabled UDP server 
 * Udp Server Version 01.1
 * Enabled wifi
 * Use of ESP_LOGI and ESP_ERR_CHECK
 * Add UDPServer
 * Add ErrorHandlings
 * Add Globals
 * Add cmp
 * Add Param
 * Add Block
 * 
 * 
 * Early code inspired by Waterbot project of Joel Parker 2017
 */

#include "Aquarium.h"
#include "ErrorHandling.h"
#include "Wifi.h"
#include "UDPServer.h"
#include "Globals.h"
#include "Timer.h"
#include "DigIO.h"
#include "Adc.h"
#include "Pwm.h"
#include "Ultrasound.h"
#include "InOut.h"
#include "Limits.h"
#include "Ota.h"

#include "freertos/event_groups.h"

static char tag[]="AQUARIUM";

int val_P02=1;


void app_main()
{
    //xTaskCreate(&waterbot_task, "waterbot_task", 4096, NULL, 5, NULL);
	
	ESP_LOGI(tag,"\n---------------------------\nVersion 3.1\n---------------------------\n");

	vTaskDelay(2000 / portTICK_PERIOD_MS);

	//INIZIALIZE IN-OUT
	dig_init(NULL);

	//INITIALIZE PERIPHERALS
	adc_init(NULL);
	pwm_init(NULL);

	//WARNIG: PWM1 AND OUT3 WORKS WITH NEGATIVE LOGIC!
	//FIRST THING TO DO IS SET HIGH THEIR VALUES
	dig_out_set(3,1);
	pwm_set(1,8191);

	//INITIALIZE SOFTWARE ROUTINE FOR INTERRUPT ON ECHO
	ultra_init(NULL);

	vTaskDelay(500 / portTICK_PERIOD_MS);

	//TURN ON A STATE LED: IN-OUT CONFIG HAS BEEN COMPLETED
	int verso = 1;
	ESP_ERROR_CHECK(gpio_set_direction(2,GPIO_MODE_INPUT_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_level(2,verso));	
    
	//INITIALIZE WIFI
	wifi_event_group = xEventGroupCreate();
    wifiudp_init(NULL);
	xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);

	//INITIALIZE UDP SERVER
	xTaskCreate(&udp_server, "udp_server_task", 8192, NULL, 5, NULL);

	//INITIALIZE HTTP OTA CLIENT

    vTaskDelay(2000 / portTICK_PERIOD_MS);

	//INITIALIZE PING

	//INIT TIMERS
    init_timer(1000); //1ms

	//WHILE TRUE TO CHECK TIMERS AND UPDATE VALUES
	while(1)
		{
		if(Timer_Tick(0)) //5ms
			{
			Clear_Timer_Tick(0);
			adc_read(1,false);
			adc_read(2,false);
			//ESP_LOGI(tag,"timer 10ms");
		}
		if(Timer_Tick(3)) //500ms
			{
			Clear_Timer_Tick(3);
			//update the INOUTData struct every 500 ms
			printf("\nSTART UPDATING IN-OUT\n");
			UpdateINOUTData(true);
			CheckLimits();		
			verso ^=1;
			ESP_ERROR_CHECK(gpio_set_level(2,verso));

		}
		if(Timer_Tick(5)) //10s
			{
			Clear_Timer_Tick(5);
			//check connection with the router
			ping_gateway(NULL);
			//ESP_LOGI(tag, "10s");
			//check ota updates
			ota_run_update(NULL);
		}
			
	}
}

