#include "commom.h"

#define GPIO_INPUT GPIO_NUM_14
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT)
#define ESP_INTR_FLAG_DEFAULT 0 // Verificar necessidade

SemaphoreHandle_t xSemaphore_capture;
QueueHandle_t buffer;

typedef struct {
  uint8_t *buf;          
  size_t len;              
}img_data;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        printf("Last error %s: 0x%x\n", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    //printf("Event dispatched from event loop base=%s, event_id=%d\n", base, event_id);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {

    case MQTT_EVENT_CONNECTED:
        printf("MQTT_EVENT_CONNECTED\n");
        msg_id = esp_mqtt_client_subscribe(client, "topic/resp", 1);
        printf("sent subscribe successful, msg_id=%d\n", msg_id);
        gpio_intr_enable(GPIO_INPUT);;
        break;

    case MQTT_EVENT_DISCONNECTED:
        printf("MQTT_EVENT_DISCONNECTED\n");
        gpio_intr_disable(GPIO_INPUT);
        break;

    case MQTT_EVENT_DATA:
        printf("MQTT_EVENT_DATA\n");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;

    case MQTT_EVENT_ERROR:
        printf("MQTT_EVENT_ERROR\n");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            printf("Last errno string (%s)\n", strerror(event->error_handle->esp_transport_sock_errno));
            printf("Other event id:%d\n", event->event_id);
        }
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
        .password = "DMK178qtS"
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, MQTT_EVENT_ERROR, mqtt_event_handler, NULL);
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
        xQueueSendToFront(buffer, &img, pdMS_TO_TICKS(0));
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
  int msg_id; 
  while (true)
  {
    if(xQueueReceive(buffer, &img_send, portMAX_DELAY) == pdTRUE)
    {
        {
            //Publica a imagem no broker
            msg_id = esp_mqtt_client_publish(client, "topic/img", (const char *)img_send.buf, img_send.len, 1, 0);
        }
    }
  }
}

static void start_wifi(){
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
}

static void init_cam(){
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
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.grab_mode = CAMERA_GRAB_LATEST;

    // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
    //                      for larger pre-allocated frame buffer.
 
    if(psramFound()){
        //printf("PSRAM found\n");
        config.frame_size = FRAMESIZE_SVGA; //FRAMESIZE_UXGA para maior resolução
        config.jpeg_quality = 10;
        config.fb_count = 2; // Modo de captura contínua
    } else {
        //printf("PSRAM not found\n");
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }
    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        //Serial.println("Camera init failed with error 0x%x", err);
        return;
    }

    sensor_t * s = esp_camera_sensor_get();
    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
      s->set_vflip(s, 1); // flip it back
      s->set_brightness(s, 1); // up the brightness just a bit
      s->set_saturation(s, -2); // lower the saturation
    }
    s->set_framesize(s, FRAMESIZE_SVGA);
    free(s);
}

void setup(void)
{
    Serial.begin(115200);
    init_cam(); // Inicializa a câmera
    start_wifi(); // Iicializa o WIFI
    configure_pins(); // Configura os pinos para interrupção de captura
    xSemaphore_capture = xSemaphoreCreateBinary();
    buffer = xQueueCreate(10, sizeof(img_data));//crea la cola *buffer* con 10 slots de 4 Bytes
    xTaskCreatePinnedToCore(capture, "capture", 8192, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(send, "send", 8192, NULL, 4, NULL, 1);
}

void loop()
{

}