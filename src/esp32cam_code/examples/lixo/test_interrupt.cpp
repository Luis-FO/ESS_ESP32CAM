#include "esp_camera.h"
#include <WiFi.h>

#include <string.h>
#include <stdlib.h>

#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_tls.h"
#include "esp_http_client.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

SemaphoreHandle_t xSemaphore_capture;

#define GPIO_INPUT GPIO_NUM_15
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT)
#define ESP_INTR_FLAG_DEFAULT 0 // Verificar necessidade
volatile int cnt = 0;

static void IRAM_ATTR isr(void* arg){
  //Explicação sobre xHigherPriorityTaskWoken:
  // https://www.freertos.org/a00124.html 
  cnt++;
  static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(xSemaphore_capture, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void capture(void *paremeter)
{
  while(true)
  {
    if(xSemaphoreTake(xSemaphore_capture, portMAX_DELAY) == pdTRUE)
    {
        Serial.println(cnt);
    }
  }
}


void configure_pins(){
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_POSEDGE;
  io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_conf);
  //install gpio isr service
  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  //hook isr handler for specific gpio pin
  gpio_isr_handler_add(GPIO_INPUT, isr, (void*)GPIO_INPUT);
  printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  configure_pins(); // Configura os pinos para interrupção de captura
  xSemaphore_capture = xSemaphoreCreateBinary(); // semaforo que libera a captura
  xTaskCreatePinnedToCore(capture, "tarea1", 4096, NULL, 0, NULL, 0);
}

void loop() {
}
