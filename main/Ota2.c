#if false

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <esp_log.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include <string.h>
#include <netdb.h>

#include "freertos/event_groups.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_ota_ops.h"

#include "nvs.h"
#include "nvs_flash.h"

#define EXAMPLE_SERVER_IP   "172.16.9.10"//"192.168.1.103"
#define EXAMPLE_SERVER_PORT "8089"
#define EXAMPLE_FILENAME "AQUARIUM.bin"
#define BUFFSIZE 1024
#define TEXT_BUFFSIZE 1024

static const char *tag = "ota";
/*an ota data write buffer ready to write to the flash*/
static char ota_write_data[BUFFSIZE + 1] = { 0 };
/*an packet receive buffer*/
static char text[BUFFSIZE + 1] = { 0 };
/* an image total length*/
static int binary_file_length = 0;
/*socket id*/
static int socket_id = -1;


static int read_until(char *buffer, char delim, int len){
    int i = 0;
    while (buffer[i] != delim && i < len) {
        ++i;
    }
    return i + 1;
}

static bool read_past_http_header(char text[], int total_len, esp_ota_handle_t update_handle){
    /* i means current position */
    int i = 0, i_read_len = 0;
    while (text[i] != 0 && i < total_len) {
        i_read_len = read_until(&text[i], '\n', total_len);
        // if we resolve \r\n line,we think packet header is finished
        if (i_read_len == 2) {
            int i_write_len = total_len - (i + 2);
            memset(ota_write_data, 0, BUFFSIZE);
            /*copy first http packet body to write buffer*/
            memcpy(ota_write_data, &(text[i + 2]), i_write_len);

            esp_err_t err = esp_ota_write( update_handle, (const void *)ota_write_data, i_write_len);
            if (err != ESP_OK) {
                ESP_LOGE(tag, "Error: esp_ota_write failed (%s)!", esp_err_to_name(err));
                return false;
            } else {
                ESP_LOGI(tag, "esp_ota_write header OK");
                binary_file_length += i_write_len;
            }
            return true;
        }
        i += i_read_len;
    }
    return false;
}

static bool connect_to_http_server(){
    ESP_LOGI(tag, "Server IP: %s Server Port:%s", EXAMPLE_SERVER_IP, EXAMPLE_SERVER_PORT);

    int  http_connect_flag = -1;
    struct sockaddr_in sock_info;

    socket_id = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_id == -1) {
        ESP_LOGE(tag, "Create socket failed!");
        esp_restart(); //WARNIG: resolve socket id error not present in new "http with https transport" esp-idf
        return false;
    }

    // set connect info
    memset(&sock_info, 0, sizeof(struct sockaddr_in));
    sock_info.sin_family = AF_INET;
    sock_info.sin_addr.s_addr = inet_addr(EXAMPLE_SERVER_IP);
    sock_info.sin_port = htons(atoi(EXAMPLE_SERVER_PORT));

    // connect to http server
    http_connect_flag = connect(socket_id, (struct sockaddr *)&sock_info, sizeof(sock_info));
    if (http_connect_flag == -1) {
        ESP_LOGE(tag, "Connect to server failed! errno=%d", errno);
        close(socket_id);
        return false;
    } else {
        ESP_LOGI(tag, "Connected to server");
        return true;
    }
    return false;
}

static int return_err(){
    ESP_LOGE(tag, "Exiting update due to fatal error...");
    close(socket_id);
    binary_file_length = 0;
    socket_id = -1;
    return 1;
}

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


    ESP_LOGI(tag, "Connect to Wifi ! Start to Connect to Server....");

    /*connect to http server*/
    if (connect_to_http_server()) {
        ESP_LOGI(tag, "Connected to http server");
    } else {
        ESP_LOGE(tag, "Connect to http server failed!");
        return return_err();
    }

    /*send GET request to http server*/
    const char *GET_FORMAT =
        "GET %s HTTP/1.0\r\n"
        "Host: %s:%s\r\n"
        "User-Agent: esp-idf/1.0 esp32\r\n\r\n";

    char *http_request = NULL;
    int get_len = asprintf(&http_request, GET_FORMAT, EXAMPLE_FILENAME, EXAMPLE_SERVER_IP, EXAMPLE_SERVER_PORT);
    if (get_len < 0) {
        ESP_LOGE(tag, "Failed to allocate memory for GET request buffer");
        return return_err();
    }
    int res = send(socket_id, http_request, get_len, 0);
    free(http_request);

    if (res < 0) {
        ESP_LOGE(tag, "Send GET request to server failed");
        return return_err();
    } else {
        ESP_LOGI(tag, "Send GET request to server succeeded");
    }

    /**/
    update_partition = esp_ota_get_next_update_partition(NULL);
    ESP_LOGI(tag, "Writing to partition subtype %d at offset 0x%x",
             update_partition->subtype, update_partition->address);
    assert(update_partition != NULL);

    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(tag, "esp_ota_begin failed (%s)", esp_err_to_name(err));
        return return_err();
    }
    ESP_LOGI(tag, "esp_ota_begin succeeded");

    bool resp_body_start = false, socket_flag = true, http_200_flag = false;
    //deal with all receive packet
    while (socket_flag) {
        memset(text, 0, TEXT_BUFFSIZE);
        memset(ota_write_data, 0, BUFFSIZE);
        int buff_len = recv(socket_id, text, TEXT_BUFFSIZE, 0);
        if (buff_len < 0) { 
            ESP_LOGE(tag, "Error: receive data error! errno=%d", errno);
            return return_err();
        } else if (buff_len > 0 && !resp_body_start) {  /*deal with response header*/
            // only start ota when server response 200 state code
            if (strstr(text, "200") == NULL && !http_200_flag) {
                ESP_LOGE(tag, "ota url is invalid or bin is not exist");
                return return_err();
            }
            http_200_flag = true;
            memcpy(ota_write_data, text, buff_len);
            resp_body_start = read_past_http_header(text, buff_len, update_handle);
        } else if (buff_len > 0 && resp_body_start) { /*deal with response body*/
            memcpy(ota_write_data, text, buff_len);
            err = esp_ota_write( update_handle, (const void *)ota_write_data, buff_len);
            if (err != ESP_OK) {
                ESP_LOGE(tag, "Error: esp_ota_write failed (%s)!", esp_err_to_name(err));
                return return_err();
            }
            binary_file_length += buff_len;
            ESP_LOGI(tag, "Have written image length %d", binary_file_length);
        } else if (buff_len == 0) {  /*packet over*/
            socket_flag = false;
            ESP_LOGI(tag, "Connection closed, all packets received");
            close(socket_id);
        } else {
            ESP_LOGE(tag, "Unexpected recv result");
        }
    }

    ESP_LOGI(tag, "Total Write binary data length : %d", binary_file_length);

    if (esp_ota_end(update_handle) != ESP_OK) {
        ESP_LOGE(tag, "esp_ota_end failed!");
        return return_err();
    }
    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(tag, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        return return_err();
    }
    ESP_LOGI(tag, "Prepare to restart system!");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    esp_restart();
    return 0;
}

#else
#endif