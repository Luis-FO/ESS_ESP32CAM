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

SemaphoreHandle_t take_picture;
QueueHandle_t buffer;

typedef struct {
  uint8_t *buf;          
  size_t len;              
}img_data;

uint8_t contagem = 0;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    printf("Event dispatched from event loop base=%s, event_id=%d\n", base, event_id);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {

    case MQTT_EVENT_CONNECTED:
        printf("MQTT_EVENT_CONNECTED\n");
        // msg_id = esp_mqtt_client_subscribe(client, "/topic/temperatura", 0);
        // printf("sent subscribe successful, msg_id=%d\n", msg_id);
        break;

    case MQTT_EVENT_DISCONNECTED:
        printf("MQTT_EVENT_DISCONNECTED\n");
        break;

    case MQTT_EVENT_ERROR:
        printf("MQTT_EVENT_ERROR\n");
        break;

    default:
        break;
    }
}

esp_mqtt_client_handle_t mqtt_app_start(void)
{
    
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://mqtt.eclipseprojects.io",
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, MQTT_EVENT_ERROR, mqtt_event_handler, NULL);
    esp_mqtt_client_register_event(client, MQTT_EVENT_DISCONNECTED, mqtt_event_handler, NULL);
    esp_mqtt_client_register_event(client, MQTT_EVENT_CONNECTED, mqtt_event_handler, NULL);
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
  img_data img_send;
  esp_mqtt_client_handle_t client = mqtt_app_start();
  int msg_id; 
  while (true)
  {
    if(xQueueReceive(buffer, &img_send, portMAX_DELAY) == pdTRUE)
    {
        //Publica a imagem no broker
        msg_id = esp_mqtt_client_publish(client, "/topic/imagem", (const char *)img_send.buf, img_send.len, 1, 0);
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
    //Função trigger para simular sinal de entrada
    const esp_timer_create_args_t esp_timer_create_args = {
        .callback = timer_callback,
        .name = "My timer"};

    esp_timer_handle_t esp_timer_handle; //handler
    esp_timer_create(&esp_timer_create_args, &esp_timer_handle);

    buffer = xQueueCreate(10, sizeof(img_data));//crea la cola *buffer* con 10 slots de 4 Bytes
    mqtt_app_start();
    xTaskCreatePinnedToCore(capture, "capture", 8192, NULL, 0, NULL, 0);

    xTaskCreatePinnedToCore(send, "send", 8192, NULL, 1, NULL, 1);
    esp_timer_start_periodic(esp_timer_handle, 1000000);
}

void loop()
{

}