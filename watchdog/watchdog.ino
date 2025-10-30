#include <esp_task_wdt.h>

#define WDT_TIMEOUT 5000  // tempo em ms (5s)

esp_task_wdt_config_t config = {
  .timeout_ms = WDT_TIMEOUT,
  .idle_core_mask = 0,   // não monitorar idle tasks
  .trigger_panic = true  // resetar sistema ao travar
};

TaskHandle_t xHandleMyTask = NULL;

void myTask(void *pvParameters) {
  while (true) {
    Serial.println("myTask rodando");
    vTaskDelay(pdMS_TO_TICKS(2000));  // simula processamento
    esp_task_wdt_reset();             // alimenta watchdog
  }
}

void setup() {
  Serial.begin(115200);
  esp_task_wdt_init(&config);  // Inicializa o watchdog com a configuração acima
  esp_task_wdt_add(NULL);      // Adiciona loopTask ao watchdog
  // Cria task separada no Core 0
  xTaskCreatePinnedToCore(myTask,          // Nome da função
                          "MyTask",        // Nome da task
                          4096,            // Tamanho da stack
                          NULL,            // Parâmetros
                          1,               // Prioridade
                          &xHandleMyTask,  // Handle da task
                          0);              // Rodar no Core 0
  // Adiciona a nova task ao watchdog
  esp_task_wdt_add(xHandleMyTask);
  Serial.println("Watchdog monitorando loopTask + myTask");
}

void loop() {
  // Simula trabalho normal
  for (int i = 0; i < 10; i++) {
    Serial.println(i);
    delay(1000);           // cada ponto = 1s → 10s total
    esp_task_wdt_reset();  // alimenta o watchdog
  }
  Serial.println("\nSimulando travamento (sem reset do WDT)...");
  delay(10000);  // maior que o timeout → watchdog vai reiniciar
}
