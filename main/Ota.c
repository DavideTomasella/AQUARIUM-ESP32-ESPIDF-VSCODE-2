#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <esp_log.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_event_loop.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"

#include "Ota.h"

#define SERVER_URL "http://192.168.1.105:8089/AQUARIUM.bin"
#define BUFFSIZE 1024

static const char *tag = "ota";
/*an ota data write buffer ready to write to the flash*/
static char ota_write_data[BUFFSIZE + 1] = { 0 };

static int http_cleanup(esp_http_client_handle_t client)
{
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return 2;
}

/*static void __attribute__((noreturn)) task_fatal_error()
{
    ESP_LOGE(tag, "Exiting task due to fatal error...");
    (void)vTaskDelete(NULL);

    while (1) {
        ;
    }
}*/

int ota_run_update(void* args){
    esp_err_t err;
    /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
    esp_ota_handle_t update_handle = 0 ;
    const esp_partition_t *update_partition = NULL;

    ESP_LOGI(tag, "Starting OTA example...");

    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (configured != running) {
        ESP_LOGW(tag, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
                 configured->address, running->address);
        ESP_LOGW(tag, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    ESP_LOGI(tag, "Running partition type %d subtype %d (offset 0x%08x)",
             running->type, running->subtype, running->address);

    esp_http_client_config_t config = {
        .url = SERVER_URL,
        //.cert_pem = (char *)server_cert_pem_start,
    };
    esp_http_client_handle_t client = NULL;
    while (1) {
        esp_get_free_heap_size();
        client = esp_http_client_init(&config);
        err = esp_http_client_open(client, 0);
        if (client == NULL) {
            ESP_LOGE(TAG, "Failed to initialise HTTP connection");
            //task_fatal_error();
        }
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
            esp_http_client_cleanup(client);
            //task_fatal_error();
        } else {
            break;
        }
        esp_get_free_heap_size();
    }
    esp_http_client_fetch_headers(client);

    update_partition = esp_ota_get_next_update_partition(NULL);
    ESP_LOGI(tag, "Writing to partition subtype %d at offset 0x%x",
             update_partition->subtype, update_partition->address);
    assert(update_partition != NULL);

    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(tag, "esp_ota_begin failed (%s)", esp_err_to_name(err));
        return http_cleanup(client);
        //task_fatal_error();
    }
    ESP_LOGI(tag, "esp_ota_begin succeeded");

    int binary_file_length = 0;
    /*deal with all receive packet*/
    while (1) {
        int data_read = esp_http_client_read(client, ota_write_data, BUFFSIZE);
        if (data_read < 0) {
            ESP_LOGE(tag, "Error: SSL data read error");
            return http_cleanup(client);
            //task_fatal_error();
        } else if (data_read > 0) {
            err = esp_ota_write( update_handle, (const void *)ota_write_data, data_read);
            if (err != ESP_OK) {
                return http_cleanup(client);
                //task_fatal_error();
            }
            binary_file_length += data_read;
            ESP_LOGD(tag, "Written image length %d", binary_file_length);
        } else if (data_read == 0) {
            ESP_LOGI(tag, "Connection closed,all data received");
            break;
        }
    }
    ESP_LOGI(tag, "Total Write binary data length : %d", binary_file_length);

    if (esp_ota_end(update_handle) != ESP_OK) {
        ESP_LOGE(tag, "esp_ota_end failed!");
        return http_cleanup(client);
        //task_fatal_error();
    }

    if (esp_partition_check_identity(esp_ota_get_running_partition(), update_partition) == true) {
        ESP_LOGI(tag, "The current running firmware is same as the firmware just downloaded");  
        return http_cleanup(client);      
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(tag, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        return http_cleanup(client);
        //task_fatal_error();
    }
    
    ESP_LOGI(tag, "Prepare to restart system!");
    esp_restart();
    
    return 0;
}