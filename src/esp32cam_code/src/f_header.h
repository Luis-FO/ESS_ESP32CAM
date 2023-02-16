#include <string.h>
#include <stdlib.h>
#include "esp_http_client.h"

void init_cam();
void start_wifi();
void capture();
esp_http_client_handle_t client_config();
void send_img();
void configure_pins();