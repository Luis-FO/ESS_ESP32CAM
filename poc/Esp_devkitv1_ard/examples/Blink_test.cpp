#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#define GPIO_OUTPUT GPIO_NUM_2
#define GPIO_OUTPUT_PIN_SEL (1ULL<<GPIO_OUTPUT)

void configure_pins(){
  gpio_config_t io_conf = {};
  io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
  io_conf.mode = GPIO_MODE_OUTPUT;
  gpio_config(&io_conf);
  gpio_set_level(GPIO_OUTPUT, 0);
}

void blink_task(void *parameter)
{
  uint32_t level = 0;
  while (true)
  {
    level = !level;
    gpio_set_level(GPIO_OUTPUT, level);
    vTaskDelay(pdMS_TO_TICKS(2000));
    printf("Pisca %d\n", level);
  }

}
void app_main(void)
{
  configure_pins();
  xTaskCreate(blink_task, "blink", 4096, NULL, 1, NULL);
}