/*********************************************************
* Exemplo que demonstra o uso de filas para comunicação
* entre tarefas. Neste exemplo, uma tarefa é responsável
* por ler o estado de um botão e enviar o estado para
* uma fila. A outra tarefa é responsável por ler o estado
* da fila e acender ou apagar um LED.
* 
* Por: Fábio Souza
* Adaptado por: Guilherme Fernandes
* Placa : Franzininho WiFi Lab01
* Data: 14/17/2025
*********************************************************/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <button.h>  // Inclui a biblioteca para manipulação de botões


//Mapa de pinos da Franzininho WiFi LAB01
// LEDs
#define LED_VERMELHO 14  // Pino do LED Vermelho
#define LED_VERDE 13     // Pino do LED Verde
#define LED_AZUL 12      // Pino do LED Azul

// Botões
#define BOTAO_1 7  // Pino do Botão 1
#define BOTAO_2 6  // Pino do Botão 2
#define BOTAO_3 5  // Pino do Botão 3
#define BOTAO_4 4  // Pino do Botão 4
#define BOTAO_5 3  // Pino do Botão 5
#define BOTAO_6 2  // Pino do Botão 6

// Buzzer
#define BUZZER 17  // Pino Do Buzzer

#define LDR 1

#define DHTPIN 15
#define DHTTYPE DHT11

// Mapeamento dos pinos
const uint8_t LED = LED_AZUL;
const uint8_t BUTTON = BOTAO_1;

// Inicializa o Button como input pull-up
Button btn(BUTTON);

static QueueHandle_t queue;

/*-------------------------------------------------------------------------------
  Tarefa para ler o estado do botão e enviar para a fila
 *---------------------------------------------------------------------------*/
void le_botao_task(void *pvParameters) {
  bool pressed = false;
  for (;;) {
    btn.update();                           // Atualiza o estado do botão
    if (btn.wasPressed()) {                 // Verifica se o botão foi pressionado
      Serial.println("Botão pressionado");  // Imprime no monitor serial
      pressed = true;
      xQueueSend(queue, &pressed, 1);  // Envia o evento para a fila
    }
  }
}

/*-------------------------------------------------------------------------------
  Tarefa para acender ou apagar o LED
  conforme o evento recebido pela fila
 *---------------------------------------------------------------------------*/
void led_task(void *pvParameters) {
  bool event, led = false;  // Variáveis para armazenar o evento e o estado do LED

  pinMode(LED, OUTPUT);    // Configura o pino do LED como saída
  digitalWrite(LED, led);  // Apaga o LED

  for (;;) {
    if (xQueueReceive(queue, &event, portMAX_DELAY) == pdTRUE) {  // Aguarda um evento da fila
      if (event) {                                                // Se o evento for verdadeiro
        Serial.println("Evento recebido");
        led = !led;              // Inverte o estado do LED
        Serial.println("LED:" + String(led ? "ON" : "OFF"));
        digitalWrite(LED, led);  // Acende ou apaga o LED
      }
    }
  }
}

  void setup() {
    Serial.begin(115200);
    queue = xQueueCreate(1, sizeof(bool));

    xTaskCreate(
      le_botao_task,    // Função da tarefa
      "Task Le Botão",  // Nome da tarefa
      2048,             // Tamaho da pilha
      NULL,             // Não usa parâmetros
      1,                // Prioridade 1
      NULL              // Não retorna o handle
    );

    xTaskCreate(
      led_task,    // Função da tarefa
      "task led",  // Nome da tarefa
      2048,        // Tamaho da pilha
      NULL,        // Não usa parâmetros
      1,           // Prioridade 1
      NULL         // Não retorna o handle
    );
  }

  void loop() {
    // Loop is free to do any other work
    vTaskDelay(pdMS_TO_TICKS(1000));  // Suspende a execução por 1 segundo
  }