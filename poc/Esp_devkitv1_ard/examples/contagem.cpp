#include <Arduino.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#define PINS 8
#define CICLOS 1

#define QTD_DIG 11 //(0-9)
#define QTD_CAPT (QTD_DIG*CICLOS)

//GPIO_NUM_33 comand capture
gpio_num_t LEDs [] = {GPIO_NUM_13, GPIO_NUM_12, GPIO_NUM_14, GPIO_NUM_27, GPIO_NUM_26, GPIO_NUM_25, GPIO_NUM_33, GPIO_NUM_2};

//Last bit is the command capture
int num [11][8] = {{1,1,1,1,1,1,0,0},
                   {0,1,1,0,0,0,0,0},
                   {1,1,0,1,1,0,1,0},
                   {1,1,1,1,0,0,1,0},
                   {0,1,1,0,0,1,1,0},
                   {1,0,1,1,0,1,1,0},
                   {1,0,1,1,1,1,1,1},
                   {1,1,1,0,0,0,0,0},
                   {1,1,1,1,1,1,1,0},
                   {1,1,1,1,0,1,1,0},
                   {0,0,0,0,0,0,0,0}
                  };

void configure_pins(){
  gpio_config_t io_conf = {};
  for (int i = 0; i<PINS; i++) 
  {
    io_conf.pin_bit_mask = (1ULL<<LEDs[i]);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;// Work without this
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;// Work without this
    gpio_config(&io_conf);
    gpio_set_level(LEDs[i], 0);
  }
}

void set_number(int pos)
{
  for (int i = 0; i<PINS; i++)
  {
    gpio_set_level(LEDs[i], num[pos][i]);
  } 

}

void setup() {
  configure_pins();
  vTaskDelay(pdMS_TO_TICKS(5000));
}

/*Novo teste:
Para cada delay serão executados x ciclos de contagem completos (0-9)
Aqui todos os dígitos serão capturados.
*/
void loop() {
  
  int times[] = {200, 100, 50, 40, 30, 10};
  int len_times = sizeof(times)/sizeof(int);
  int num = 0;


  for(int i = 0; i<len_times;i++)
  {
    for(int j = 0;j<QTD_CAPT;j++)
    {
      vTaskDelay(pdMS_TO_TICKS(times[i]));
      num = j%QTD_DIG; // Operação resto para obter sempre um número entre 0 e 9
      set_number(num);
    }
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
  
}