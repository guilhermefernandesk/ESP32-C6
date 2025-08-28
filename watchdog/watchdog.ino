#include <esp_task_wdt.h>
// #include "soc/rtc_wdt.h"

#define WDT_TIMEOUT 5000  // tempo em ms

esp_task_wdt_config_t config = {
  .timeout_ms = WDT_TIMEOUT,
  .idle_core_mask = 0,   // não monitorar idle tasks
  .trigger_panic = true  // resetar sistema ao travar
};

TaskHandle_t xHandleMyTask = NULL;

void myTask(void *pvParameters) {
  while (true) {
    Serial.println("Task rodando...");
    vTaskDelay(pdMS_TO_TICKS(2000));  // simula processamento
    esp_task_wdt_reset();             // alimenta watchdog
  }
}

void setup() {
  Serial.begin(115200);

  esp_task_wdt_init(&config);  // Inicializa o watchdog com a configuração acima
  esp_task_wdt_add(NULL);      // Adiciona loopTask ao watchdog

  // Cria task separada no Core 0
  xTaskCreatePinnedToCore(
    myTask,          // Nome da função
    "MyTask",        // Nome da task
    4096,            // Tamanho da stack
    NULL,            // Parâmetros
    1,               // Prioridade
    &xHandleMyTask,  // Handle da task
    0);              // Rodar no Core 0

  // Adiciona a nova task ao watchdog
  esp_task_wdt_add(xHandleMyTask);

  Serial.println("Watchdog monitorando loopTask + MyTask");

  // rtc_wdt_protect_off();                   // desliga proteção
  // rtc_wdt_disable();                       // garante desativado
  // rtc_wdt_set_time(RTC_WDT_STAGE0, 5000);  // timeout 5s
  // rtc_wdt_set_stage(RTC_WDT_STAGE0, RTC_WDT_STAGE_ACTION_RESET_SYSTEM);
  // rtc_wdt_enable();
}

void loop() {
  // Simula trabalho normal
  for (int i = 0; i < 10; i++) {
    Serial.print(".");
    delay(400);            // cada ponto = 0.4s → 4s total
    esp_task_wdt_reset();  // alimenta o watchdog
    // rtc_wdt_feed();        // alimenta watchdog rtc
  }

  Serial.println("\nSimulando travamento (sem reset do WDT)...");
  delay(6000);  // maior que o timeout → watchdog vai reiniciar
}
