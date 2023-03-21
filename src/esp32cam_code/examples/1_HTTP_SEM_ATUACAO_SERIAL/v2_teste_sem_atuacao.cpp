#include "commom.h"

#define MAX_HTTP_OUTPUT_BUFFER 2048

#define GPIO_INPUT GPIO_NUM_14
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT)
#define ESP_INTR_FLAG_DEFAULT 0 // Verificar necessidade


SemaphoreHandle_t xSemaphore_capture;
QueueHandle_t buffer;

static void init_cam(int aec_value, int agc_gain, framesize_t framesize){
    //printf("Cam Init: Running\n");
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
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.grab_mode = CAMERA_GRAB_LATEST;
    config.frame_size = framesize; //FRAMESIZE_CIF; //FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        return;
    }
    
    sensor_t * s = esp_camera_sensor_get();

    s->set_exposure_ctrl(s, 0);
    s->set_aec_value(s, aec_value);
    
    s->set_gain_ctrl(s, 0);
    s->set_agc_gain(s, agc_gain);

}

void start_wifi(){
  //printf("Start WIFI: Running\n");
  const char* ssid = "VIVOFIBRA-0E70";
  const char* password = "5246D8B9B9";
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

static void IRAM_ATTR isr(void* arg){
  static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(xSemaphore_capture, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void capture(void *paremeter)
{
  camera_fb_t *fb = NULL;
  img_data img;
  while(true)
  {
    if(xSemaphoreTake(xSemaphore_capture, portMAX_DELAY) == pdTRUE)
    {
      fb = esp_camera_fb_get();
      if (!fb){

      }
      else{
        img.buf = (uint8_t *)malloc(fb->len);
        memcpy(img.buf, fb->buf, fb->len);
        img.len = fb->len;
        xQueueSendToFront(buffer, &img, pdMS_TO_TICKS(0));
        free(img.buf);
      }
      esp_camera_fb_return(fb);
    }
  }
}

void send(void *parameter){
  
  //char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
  esp_err_t err;
  esp_http_client_config_t config = {0};
  config.url = "http://192.168.15.12:5000/api/test";
  //config.event_handler = _http_event_handler;
  //config.user_data = local_response_buffer;
  config.method = HTTP_METHOD_POST;
  esp_http_client_handle_t client = esp_http_client_init(&config);
  img_data img_send;

  while (true)
  {
    if(xQueueReceive(buffer, &img_send, portMAX_DELAY) == pdTRUE)
    {

      esp_http_client_set_header(client, "content-type", "image/jpeg");
      esp_http_client_set_post_field(client, (const char *)img_send.buf, img_send.len);
      err = esp_http_client_perform(client);

    }

  }
  esp_http_client_cleanup(client);
}

void configure_pins(){
  //printf("Pin configuration: Running\n");
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_POSEDGE;
  io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_conf);
  //install gpio isr service
  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT); //A proxima linhas dispensa essa
  //hook isr handler for specific gpio pin
  gpio_isr_handler_add(GPIO_INPUT, isr, (void*)GPIO_INPUT);
  //printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
  gpio_intr_disable(GPIO_INPUT);
  //printf("Pause ISR for 5s\n");
  vTaskDelay(pdMS_TO_TICKS(5000));
  gpio_intr_enable(GPIO_INPUT);

}

int SerialRead(Indexed_Data *values){
  
  Serial.begin(115200);
  String input;
  while(true){
    if (Serial.available() > 0) {
      input = Serial.readString();

      values[0].index = input.indexOf("-");
      values[1].index = input.indexOf("-", values[0].index+1);

      values[0].value = input.substring(0, values[0].index).toInt();
      values[1].value = input.substring(values[0].index+1, values[1].index).toInt(); 
      values[2].value = input.substring(values[1].index+1).toInt();
      
      Serial.println(values[0].value);
      Serial.println(values[1].value);
      Serial.println(values[2].value);
      //Encerra a serial ao final da leitura
      Serial.flush();
      Serial.end();
      return 1;
      
    }
  }
  // Encerra a Serial em caso de eventual falha
  Serial.end();
  return -1;

}

void setup() {

  Indexed_Data c_settings[3];
  SerialRead(c_settings);
  // Pos: 
  // 0 -> aec_value ; 1 -> agc_gain; 2 -> framesize.
  init_cam(c_settings[0].value, c_settings[1].value, (framesize_t)c_settings[2].value); // Inicializa a câmera
  
  //init_cam(); // Inicializa a câmera
  start_wifi(); // Iicializa o WIFI
  configure_pins(); // Configura os pinos para interrupção de captura

  xSemaphore_capture = xSemaphoreCreateBinary(); // semaforo que libera a captura
  buffer = xQueueCreate(10, sizeof(img_data));//crea la cola *buffer* con 10 slots de 4 Bytes

  // xTaskCreatePinnedToCore(capture, "capture", 8192, NULL, 4, NULL, 0);
  // xTaskCreatePinnedToCore(send, "send", 8192, NULL, 2, NULL, 1);
  xTaskCreate(capture, "capture", 8192, NULL, 4, NULL); // Maior prioridade
  xTaskCreate(send, "send", 8192, NULL, 2, NULL);
}

void loop() {
}
