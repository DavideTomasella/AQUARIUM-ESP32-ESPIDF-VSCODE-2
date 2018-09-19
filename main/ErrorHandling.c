#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <errno.h>
#include <sys/socket.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "ErrorHandling.h"

static char tag[]="error_handling";

int get_socket_error_code(int socket)
{
    int result;
    u32_t optlen = sizeof(int);
    if(getsockopt(socket, SOL_SOCKET, SO_ERROR, &result, &optlen) == -1) {
        ESP_LOGE(tag, "getsockopt failed");
        return -1;
    }
    return result;
    
}

int show_socket_error_reason(int socket)
{
    int err = get_socket_error_code(socket);
    ESP_LOGW(tag, "socket error %d %s", err, strerror(err));
    return err;
}

void close_socket(int socket)
{
    close(socket);
    ESP_LOGI(tag, "my socket closed");
}

