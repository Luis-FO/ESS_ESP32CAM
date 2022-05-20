/*O programa executa 2 tarefas em pararelo no mesmo núcleo
 *Devem ser impressos os numeros de 0 até 14
 * A cada contagem completa a tarefa 1 pausa por 5 segundo 
 * Enquado a tarefa 2 imprime erros.
 */

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_system.h>
#include <esp_log.h>

QueueHandle_t buffer;

void tarefa_1(void*arg)
{
    uint32_t snd = 0;
    while(1)
    {
        if (snd < 15)
        {
            //Envia para fila
            xQueueSend(buffer, &snd, pdMS_TO_TICKS(0));
            snd++;
        }
        else
        {
            //Espera por 5 segundos 
            vTaskDelay(pdMS_TO_TICKS(5000));
            // Reseta a variável snd
            snd = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void tarefa_2(void*z)
{
    uint32_t rcv = 0;
    while(1)
    {
        //Se dentro de 1 segundo receber algo, imprime o valor
        if (xQueueReceive(buffer, &rcv, pdMS_TO_TICKS(1000)) == true)
        {
            //Imprime o valor recebido
            ESP_LOGI("Cola", "Item recibido: %u", rcv);
        }
        else
        {
            //Se ultrapassar o tempo, imprime erro
            ESP_LOGE("Cola", "Item no recebido, timeout expiro!");
        }
    }
}

void app_main()
{
    //Aloca 10 slots de 4 bytes para o buffer
    buffer = xQueueCreate(10, sizeof(uint32_t));

    xTaskCreatePinnedToCore(tarefa_1, "tarefa_1", 4096, NULL, 1, NULL, 0);//Cria tarefa que envia
    xTaskCreatePinnedToCore(tarefa_2, "tarefa_2", 4096, NULL, 1, NULL, 1);//Cria tarefa que recebe.
}
