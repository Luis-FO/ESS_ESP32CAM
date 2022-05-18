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


// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size


#define CAMERA_MODEL_AI_THINKER // Has PSRAM

#include "camera_pins.h"

#define MAX_HTTP_OUTPUT_BUFFER 2048
#define GPIO_OUTPUT GPIO_NUM_14
#define GPIO_OUTPUT_PIN_SEL (1ULL<<GPIO_OUTPUT)

#define GPIO_INPUT GPIO_NUM_15
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT)
#define ESP_INTR_FLAG_DEFAULT 0 // Verificar necessidade

uint32_t sinal = 0;
volatile bool trigger = false;
const char* ssid = "VIVOFIBRA-0E70";
const char* password = "5246D8B9B9";
camera_fb_t *fb = NULL;

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
  static char *output_buffer;  // Buffer to store response of http request from event handler
  static int output_len;       // Stores number of bytes read
  switch(evt->event_id) {
      case HTTP_EVENT_ERROR:
          ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
          break;
      case HTTP_EVENT_ON_CONNECTED:
          ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
          break;
      case HTTP_EVENT_HEADER_SENT:
          ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
          break;
      case HTTP_EVENT_ON_HEADER:
          ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
          break;
      case HTTP_EVENT_ON_DATA:
          ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
          /*
           *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
           *  However, event handler can also be used in case chunked encoding is used.
           */
          if (!esp_http_client_is_chunked_response(evt->client)) {
              // If user_data buffer is configured, copy the response into the buffer
              if (evt->user_data) {
                  memcpy(evt->user_data + output_len, evt->data, evt->data_len);
              } else {
                  if (output_buffer == NULL) {
                      output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
                      output_len = 0;
                      if (output_buffer == NULL) {
                          ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                          return ESP_FAIL;
                      }
                  }
                  memcpy(output_buffer + output_len, evt->data, evt->data_len);
              }
              output_len += evt->data_len;
          }

          break;
      case HTTP_EVENT_ON_FINISH:
          ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
          if (output_buffer != NULL) {
              // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
              // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
              free(output_buffer);
              output_buffer = NULL;
          }
          output_len = 0;
          break;
  }
  return ESP_OK;
}

void init_cam()
{
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_SVGA);
  
}

void start_wifi()
{
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

static void IRAM_ATTR capture(void* arg){
  trigger = true;
}

int send_img(){
  Serial.println("Enviando imagem");
  fb = esp_camera_fb_get();
  if (!fb){
      Serial.println("Camera capture failed");
      return ESP_FAIL;
    }
  
  char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
  esp_err_t err;
  esp_http_client_config_t config = {0};
  config.url = "http://192.168.15.12:5000/api/test";
  config.event_handler = _http_event_handler;
  config.user_data = local_response_buffer;

  esp_http_client_handle_t client = esp_http_client_init(&config);
  esp_http_client_set_method(client, HTTP_METHOD_POST);

  esp_http_client_set_header(client, "content-type", "image/jpeg");
  esp_http_client_set_post_field(client, (const char *)fb->buf, fb->len);

  err = esp_http_client_perform(client);
  Serial.print("Média: ");
  Serial.println(local_response_buffer);
  esp_camera_fb_return(fb);

  esp_http_client_cleanup(client);
}

void configure_pins(){
  //Configura Saída pino 14
  //zero-initialize the config structure.
  gpio_config_t io_conf = {};
  //disable interrupt
  io_conf.intr_type = GPIO_INTR_DISABLE;
  //set as output mode
  io_conf.mode = GPIO_MODE_OUTPUT;
  //bit mask of the pins that you want to set
  io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
  //disable pull-down mode
  io_conf.pull_down_en = gpio_pulldown_t(0);
  //disable pull-up mode
  io_conf.pull_up_en = gpio_pullup_t(0);
  //configure GPIO with the given settings
  gpio_config(&io_conf);

  //Configura Entrada pino 15
  //Com interrupção configurada na borda de subida do pino
  //interrupt of rising edge
  io_conf.intr_type = GPIO_INTR_POSEDGE;
  //bit mask of the pins, use GPIO4/5 here
  io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
  //set as input mode
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_down_en = gpio_pulldown_t(1);
  gpio_config(&io_conf);

  //install gpio isr service
  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  //hook isr handler for specific gpio pin
  gpio_isr_handler_add(GPIO_INPUT, capture, (void*) GPIO_INPUT);
  //printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  init_cam();
  start_wifi();
  configure_pins();
}

void loop() {
  
  if(trigger==true){
    send_img();
    trigger = false;
  }
  else if (sinal==0){
     sinal = 1;
     gpio_set_level(gpio_num_t(GPIO_OUTPUT), sinal);
     //Serial.println("invert1");
  }
  else{
     sinal = 0;
     gpio_set_level(gpio_num_t(GPIO_OUTPUT), sinal);
     //Serial.println("invert2");
  }
  vTaskDelay(1000/portTICK_RATE_MS);
}
