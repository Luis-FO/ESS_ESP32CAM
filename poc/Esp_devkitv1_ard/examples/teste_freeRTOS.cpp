#include <Arduino.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#define GPIO_INPUT GPIO_NUM_19
#define ESP_INTR_FLAG_DEFAULT 0

SemaphoreHandle_t xSemaphore_capture;
QueueHandle_t buffer;
int contador = 0;

typedef struct{
  int *buf;
  int len;
}DATA;

//Ponteiro que aponta para a contagem
DATA *cnt = NULL;

DATA *retorna_contagem()
{
  if (contador>=50)
  {
    contador = 0;
  }
  cnt = (DATA *)malloc(sizeof(DATA)); //Aloca espaço na memória do tamando de DATA para cnt
  cnt->len = sizeof(int);
  cnt->buf = &contador;
  contador++;
  return cnt;
}

void libera_fb(DATA *dado)
{
  //Libera o espaço de memória alocada para cnt
  free(cnt);
}

void Task_main(void *parametros)
{ 
  //Esperar 10s no inicio
  vTaskDelay(pdMS_TO_TICKS(10000));
  while(true)
  {
    //Entrega o Semaforo a cada 1s
    xSemaphoreGive(xSemaphore_capture);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  
}

void Task_A(void *parametros)
{
  //img a nossa estrutura local
  //DATA img;
  //Ponteiro que apontará para nossa espaço de memória cnt
  DATA *fb;
  DATA img;
  img.buf = NULL;
  img.len;
  while (true)
  {
    if(xSemaphoreTake(xSemaphore_capture, portMAX_DELAY) == pdTRUE)
    { 
      fb = retorna_contagem();
      img.buf = (int *)malloc(fb->len);
      //pvPortMalloc
      memcpy(img.buf, fb->buf, fb->len);
      img.len = fb->len;
      xQueueSend(buffer, &img, pdMS_TO_TICKS(0));
      libera_fb(fb);
      free(img.buf);
    }
  }
}

void Task_B(void *parametros)
{
  DATA img_receive;
  while (true)
  {
    if(xQueueReceive(buffer, &img_receive, portMAX_DELAY) == pdTRUE)
    {
      //Imprime valor na tela
      //vTaskDelay(pdMS_TO_TICKS(1000));
      printf("Receive from task A: %d\n", *img_receive.buf); // This work
      printf("Contador: %d\n", contador);
    }
  }
}


void setup() {
  Serial.begin(115200);
  xSemaphore_capture = xSemaphoreCreateBinary();
  buffer = xQueueCreate(10, sizeof(DATA));//crea la cola *buffer* con 10 slots de 4 Bytes
  
  xTaskCreatePinnedToCore(Task_main, "Task_main", 8192, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(Task_A, "Task_A", 8192, NULL, 3, NULL, 0);
  xTaskCreatePinnedToCore(Task_B, "Task_B", 8192, NULL, 4, NULL, 1);
}

void loop() {
  
  
}