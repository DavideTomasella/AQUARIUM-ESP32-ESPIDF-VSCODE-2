#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <esp_log.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "freertos/queue.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "esp_types.h"

#include "Timer.h"
#include "Globals.h"

static char tag[]="timer";

static int SW_Timer[NUM_TIMER];
static int SW_Timer_Reload[NUM_TIMER]={5,10,100,500,1000,10000};
static char SW_Timer_flag[NUM_TIMER];

static intr_handle_t s_timer_handle;

static void timer_isr(void* arg)
{
    int i;
    TIMERG0.int_clr_timers.t0 = 1;
    TIMERG0.hw_timer[0].config.alarm_en = 1;

    for(i=0;i<NUM_TIMER;i++)
    	if(!SW_Timer[i]--) {
            SW_Timer[i]=SW_Timer_Reload[i];
            SW_Timer_flag[i]=1;
        }
    
    // your code, runs in the interrupt
}

char Timer_Tick(unsigned char t) {
    return(SW_Timer_flag[t]);
}

void Clear_Timer_Tick(unsigned char t) {
    SW_Timer_flag[t]=0;
}

void init_timer(int timer_period_us)
{
    timer_config_t config = {
            .alarm_en = true,
            .counter_en = true,
            .intr_type = TIMER_INTR_LEVEL,
            .counter_dir = TIMER_COUNT_UP,
            .auto_reload = true,
            .divider = 80   /* 1 us per tick */
    };
    ESP_LOGI(tag, "TIMER_0 initialing");
    ESP_ERROR_CHECK(timer_init(TIMER_GROUP_0, TIMER_0, &config));
    ESP_ERROR_CHECK(timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0));
    ESP_ERROR_CHECK(timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, timer_period_us));
    ESP_ERROR_CHECK(timer_enable_intr(TIMER_GROUP_0, TIMER_0));
    ESP_ERROR_CHECK(timer_isr_register(TIMER_GROUP_0, TIMER_0, &timer_isr, NULL, 0, &s_timer_handle));
    
    ESP_LOGI(tag, "TIMER_0 starting");
    ESP_ERROR_CHECK(timer_start(TIMER_GROUP_0, TIMER_0));
    ESP_LOGI(tag, "TIMER_0 initialized\n");
}