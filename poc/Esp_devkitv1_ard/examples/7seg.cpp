#include <Arduino.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#define PINS 8

//GPIO_NUM_33 comand capture
gpio_num_t LEDs [] = {GPIO_NUM_13, GPIO_NUM_12, GPIO_NUM_14, GPIO_NUM_27, GPIO_NUM_26, GPIO_NUM_25, GPIO_NUM_33, GPIO_NUM_2};

//Last bit is the command capture
int num [11][8] = {{1,1,1,1,1,1,0,0},
                   {0,1,1,0,0,0,0,0},
                   {1,1,0,1,1,0,1,0},
                   {1,1,1,1,0,0,1,0},
                   {0,1,1,0,0,1,1,0},
                   {1,0,1,1,0,1,1,0},
                   {1,0,1,1,1,1,1,0},
                   {1,1,1,0,0,0,0,1},
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
}

void loop() {
  int times[] = {2000, 1500, 1000, 500, 200, 100, 50, 10, 1};

  for(int i = 0; i<9;i++)
  {
    for(int j = 0;j<11;j++)
    {
      vTaskDelay(pdMS_TO_TICKS(times[i]));
      set_number(j);
    }
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
  
  
}