#include <Arduino.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

gpio_num_t LEDs [] = {GPIO_NUM_13, GPIO_NUM_12, GPIO_NUM_14, GPIO_NUM_27, GPIO_NUM_26, GPIO_NUM_25, GPIO_NUM_33};


int num [10][7] = {{1,1,1,1,1,1,0},
                   {0,1,1,0,0,0,0},
                   {1,1,0,1,1,0,1},
                   {1,1,1,1,0,0,1},
                   {0,1,1,0,0,1,1},
                   {1,0,1,1,0,1,1},
                   {1,0,1,1,1,1,1},
                   {1,1,1,0,0,0,0},
                   {1,1,1,1,1,1,1},
                   {1,1,1,1,0,1,1}
                  };
 

void configure_pins(){
  gpio_config_t io_conf = {};
  for (int i = 0; i<7; i++) 
  {
    io_conf.pin_bit_mask = (1ULL<<LEDs[i]);
    io_conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&io_conf);
    gpio_set_level(LEDs[i], 0);
  }
}

void set_number(int pos)
{
  for (int i = 0; i<7; i++)
  {
    gpio_set_level(LEDs[i], num[pos][i]);
  } 

}

void setup() {
  configure_pins();
}

void loop() {
  for(int i = 0;i<10;i++)
  {
    set_number(i);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}