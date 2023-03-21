#include "commom.h"
#include "cJSON.h"

#define GPIO_INPUT GPIO_NUM_14
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT)
#define ESP_INTR_FLAG_DEFAULT 0 // Verificar necessidade

SemaphoreHandle_t xSemaphore_capture;
QueueHandle_t buffer;
QueueHandle_t cam_config_buffer;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    //printf("Event dispatched from event loop base=%s, event_id=%d\n", base, event_id);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    cam_config conf = {};
    
    switch ((esp_mqtt_event_id_t)event_id) {

    case MQTT_EVENT_CONNECTED:
        //printf("MQTT_EVENT_CONNECTED\n");
        //msg_id = esp_mqtt_client_subscribe(client, "topic/resp", 1);
        esp_mqtt_client_subscribe(client, "topic/get_image", 1);
        esp_mqtt_client_subscribe(client, "topic/cam_config", 1);
        //printf("sent subscribe successful, msg_id=%d\n", msg_id);
        //gpio_intr_enable(GPIO_INPUT);
        break;

    case MQTT_EVENT_DISCONNECTED:
        //printf("MQTT_EVENT_DISCONNECTED\n");
        //msg_id = esp_mqtt_client_unsubscribe(client, "topic/resp");
        esp_mqtt_client_unsubscribe(client, "topic/get_image");
        esp_mqtt_client_unsubscribe(client, "topic/cam_config");
        //gpio_intr_disable(GPIO_INPUT);
        break;

    case MQTT_EVENT_DATA:
        // printf("MQTT_EVENT_DATA\n");
        //printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        //printf("Tam=%i\n", event->topic_len);
        // printf("DATA=%.*s\r\n", event->data_len, event->data);
        conf.topic = event->topic;
        conf.data = event->data;
        xQueueSend(cam_config_buffer, &conf, pdMS_TO_TICKS(0));
        //xSemaphoreGive(xSemaphore_capture);
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
        .password = "DMK178qtS",
        .out_buffer_size = 100000//verificar necessidade
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    //esp_mqtt_client_register_event(client, MQTT_EVENT_ERROR, mqtt_event_handler, NULL);
    esp_mqtt_client_register_event(client, MQTT_EVENT_DISCONNECTED, mqtt_event_handler, NULL);
    esp_mqtt_client_register_event(client, MQTT_EVENT_CONNECTED, mqtt_event_handler, NULL);
    esp_mqtt_client_register_event(client, MQTT_EVENT_DATA, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    return client;
}

static void IRAM_ATTR isr(void* arg){
  //Explicação sobre xHigherPriorityTaskWoken:
  // https://www.freertos.org/a00124.html 
  static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(xSemaphore_capture, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void interpret_data(void *parameter)
{
  cam_config data = {};
  const char *topic = "topic/cam_config";
  size_t len;

  //////setings
  cJSON *json = NULL;
  int vflip;
  int aec_value_exp;
  int agc_gain;

  framesize_t framesize;
  while(true)
  {
    if(xQueueReceive(cam_config_buffer, &data, portMAX_DELAY) == pdTRUE)
    {
        //*********************Melhorar*********************
        len = strlen(data.topic);
        char b[len+1];
        strcpy(b, data.topic);
        b[len] = '\0';
        //*********************Melhorar*********************

        if(strcmp(b, topic) == 0){
          json = cJSON_ParseWithLength(data.data, strlen(data.data));
          vflip = cJSON_GetObjectItemCaseSensitive(json, "vflip")->valueint;
          aec_value_exp = cJSON_GetObjectItemCaseSensitive(json, "aec_value")->valueint;
          agc_gain = cJSON_GetObjectItemCaseSensitive(json, "agc_gain")->valueint;
          framesize = framesize_t(cJSON_GetObjectItemCaseSensitive(json, "framesize")->valueint);
          printf("%i", aec_value_exp);
          //get sensor pointer
          sensor_t * s = esp_camera_sensor_get();
          //Flip image
          s->set_vflip(s, vflip);
          s->set_exposure_ctrl(s, 0);
          s->set_aec_value(s, aec_value_exp);
          s->set_framesize(s, framesize);
          s->set_gain_ctrl(s, 0);
          s->set_agc_gain(s, agc_gain);
          
        }
        else{
          xSemaphoreGive(xSemaphore_capture);
        }
        
    }
  }
}

void capture(void *paremeter)
{
  camera_fb_t *fb = NULL;
  img_data img;
  while(true)
  {
    if(xSemaphoreTake(xSemaphore_capture, portMAX_DELAY) == pdTRUE)
    {
      //printf("Take");
      fb = esp_camera_fb_get();
      if (!fb){
       //Serial.println("Camera capture failed");//Remover para os testes.
      }
      else{
        img.buf = (uint8_t *)malloc(fb->len); // Substituir malloc por sua versão mais adequada.
        memcpy(img.buf, fb->buf, fb->len);
        img.len = fb->len;
        xQueueSend(buffer, &img, pdMS_TO_TICKS(0));
        free(img.buf);
      }
      esp_camera_fb_return(fb);
    }
  }
}

void send(void *args)
{  
  esp_mqtt_client_handle_t client = mqtt_app_start();
  img_data img_send;
  //int msg_id; 
  while (true)
  {
    if(xQueueReceive(buffer, &img_send, portMAX_DELAY) == pdTRUE)
    {
        {
            //Publica a imagem no broker
            esp_mqtt_client_enqueue(client, "topic/img", (const char *)img_send.buf, img_send.len, 1, 0, true); //versão não bloqueante
            //msg_id = esp_mqtt_client_publish(client, "topic/img", (const char *)img_send.buf, img_send.len, 1, 0);
            
        }
    }
  }
}

static void start_wifi(){
  //printf("Start WIFI: Running\n");
  const char* ssid = "VIVOFIBRA-0E70";
  const char* password = "5246D8B9B9";
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(pdMS_TO_TICKS(500));
   //Serial.print(".");
  }
  //Serial.println("");
  //Serial.println("WiFi connected");
}

static void configure_pins(){
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
  vTaskDelay(pdMS_TO_TICKS(5000));
  gpio_intr_enable(GPIO_INPUT);
}

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
    config.frame_size = framesize;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 10;
    config.fb_count = 2;

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        return;
    }
    

    sensor_t * s = esp_camera_sensor_get();

    s->set_exposure_ctrl(s, 0);
    s->set_aec_value(s, aec_value);
    
    //s->set_framesize(s, framesize);

    
    s->set_gain_ctrl(s, 0);
    s->set_agc_gain(s, agc_gain);

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

void setup(void)
{
    // Serial.begin(115200);
    Indexed_Data c_settings[3];
    SerialRead(c_settings);
    // Pos: 
    // 0 -> aec_value ; 1 -> agc_gain; 2 -> framesize.

    init_cam(c_settings[0].value, c_settings[1].value, (framesize_t)c_settings[2].value); // Inicializa a câmera
    start_wifi(); // Iicializa o WIFI
    configure_pins(); // Configura os pinos para interrupção de captura

    xSemaphore_capture = xSemaphoreCreateBinary();
    buffer = xQueueCreate(10, sizeof(img_data));//crea la cola *buffer* con 10 slots de 4 Bytes

    cam_config_buffer = xQueueCreate(2, sizeof(cam_config));
    
    // xTaskCreatePinnedToCore(interpret_data, "interpret_data", 8192, NULL, 2, NULL, 1);
    xTaskCreate(capture, "capture", 8192, NULL, 4, NULL); // Maior prioridade
    xTaskCreate(send, "send", 8192, NULL, 2, NULL);
    xTaskCreate(interpret_data, "interpret_data", 8192, NULL, 2, NULL);
}

void loop()
{

}