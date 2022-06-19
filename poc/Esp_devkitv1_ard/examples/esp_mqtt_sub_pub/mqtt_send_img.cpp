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
#include "commom.h"

SemaphoreHandle_t take_picture;
QueueHandle_t buffer;
esp_timer_handle_t esp_timer_handle; //handler


uint8_t contagem = 0;


static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        printf("Last error %s: 0x%x\n", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    //printf("Event dispatched from event loop base=%s, event_id=%d\n", base, event_id);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {

    case MQTT_EVENT_CONNECTED:
        printf("MQTT_EVENT_CONNECTED\n");
        msg_id = esp_mqtt_client_subscribe(client, "topic/resp", 1);
        printf("sent subscribe successful, msg_id=%d\n", msg_id);
        timer_start();
        break;

    case MQTT_EVENT_DISCONNECTED:
        printf("MQTT_EVENT_DISCONNECTED\n");
        timer_stop();
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
        break;
    }
}

esp_mqtt_client_handle_t mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://192.168.15.12",
        .port = 1883,
        .username = "luis",
        .password = "DMK178qtS"
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, MQTT_EVENT_ERROR, mqtt_event_handler, NULL);
    esp_mqtt_client_register_event(client, MQTT_EVENT_DISCONNECTED, mqtt_event_handler, NULL);
    esp_mqtt_client_register_event(client, MQTT_EVENT_CONNECTED, mqtt_event_handler, NULL);
    esp_mqtt_client_register_event(client, MQTT_EVENT_DATA, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    return client;
}

img_data *get_contagem()
{
    contagem++;
    img_data *fb = (img_data *)malloc(sizeof(img_data));
    fb->len = sizeof(contagem);
    fb->buf = &contagem;
    return fb;
}

void timer_callback(void *args)
{
    static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(take_picture, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void timer_start()
{
    //Função trigger para simular sinal de entrada
    const esp_timer_create_args_t esp_timer_create_args = {
        .callback = timer_callback,
        .name = "My timer"};
    esp_timer_create(&esp_timer_create_args, &esp_timer_handle);
    esp_timer_start_periodic(esp_timer_handle, 2000000);
}

void timer_stop()
{
    esp_timer_stop(esp_timer_handle);
    esp_timer_delete(esp_timer_handle);
}

void capture(void *args){
    img_data *fb = NULL;
    img_data img;
    while(true)
    {
        if(xSemaphoreTake(take_picture, portMAX_DELAY) == pdTRUE)
        {
            fb = get_contagem();
            if (!fb){
                //Serial.println("Camera capture failed");//Remover para os testes.
            }
            else{
                img.buf = (uint8_t *)malloc(fb->len);
                memcpy(img.buf, fb->buf, fb->len);
                img.len = fb->len;
                xQueueSendToFront(buffer, &img, pdMS_TO_TICKS(0));
                free(img.buf);
            }
            free(fb);
        }
    }
}

void send(void *args)
{  
  //esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)args;
  esp_mqtt_client_handle_t client = mqtt_app_start();
  img_data img_send;
  int msg_id; 
  while (true)
  {
    if(xQueueReceive(buffer, &img_send, portMAX_DELAY) == pdTRUE)
    {
        {
            //Publica a imagem no broker
            msg_id = esp_mqtt_client_publish(client, "topic/img", (const char *)img_send.buf, img_send.len, 1, 0);
        }
    }

  }
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
    take_picture = xSemaphoreCreateBinary();
    buffer = xQueueCreate(10, sizeof(img_data));//crea la cola *buffer* con 10 slots de 4 Bytes
    xTaskCreatePinnedToCore(capture, "capture", 8192, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(send, "send", 8192, NULL, 4, NULL, 1);
}

void loop()
{

}