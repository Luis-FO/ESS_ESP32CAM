#include <Arduino.h>
#include <WiFi.h>

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "MQTT_EXAMPLE";


static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        printf("Last error %s: 0x%x\n", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    printf("Event dispatched from event loop base=%s, event_id=%d\n", base, event_id);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        printf("MQTT_EVENT_CONNECTED\n");
        msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        printf("sent publish successful, msg_id=%d\n", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        printf("sent subscribe successful, msg_id=%d\n", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        printf("sent subscribe successful, msg_id=%d\n", msg_id);

        msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        printf("sent unsubscribe successful, msg_id=%d\n", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        printf("MQTT_EVENT_DISCONNECTED\n");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        printf("MQTT_EVENT_SUBSCRIBED, msg_id=%d\n", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        printf("sent publish successful, msg_id=%d\n", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        printf("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d\n", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        printf("MQTT_EVENT_PUBLISHED, msg_id=%d\n", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        printf("MQTT_EVENT_DATA\n");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        printf("MQTT_EVENT_ERROR\n");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            printf("Last errno string (%s)\n", strerror(event->error_handle->esp_transport_sock_errno));
            printf("Other event id:%d\n", event->event_id);

        }
        break;
    default:
        printf("Other event id:%d\n", event->event_id);
        break;
    }
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://mqtt.eclipseprojects.io",
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void start_wifi(){
  printf("Start WIFI: Running\n");
  const char* ssid = "VIVOFIBRA-0E70";
  const char* password = "5246D8B9B9";
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(pdMS_TO_TICKS(500));
   Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void setup(void)
{
    Serial.begin(115200);
    start_wifi();
    mqtt_app_start();
}

void loop()
{

}