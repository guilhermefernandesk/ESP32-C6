/*
    Placa Franzininho WiFi Lab01
*/

// Bibliotecas necessárias
#include <WiFi.h>
#include <esp_now.h>
#include "button.h"

#define LED_VERDE 13  // Pino do LED Verde
#define BOTAO_1 7     // Pino do Botão 1
#define ID 1      // Identificador unico

Button button(BOTAO_1);  // Cria classe button

//mac address da placa (troque para o mac da sua placa)
uint8_t macAddress[] = { 0x40, 0x4C, 0xCA, 0x56, 0xB6, 0x74 };

// Estrutura para enviar dados e receber
typedef struct struct_message {
  uint8_t botaoId;
  uint8_t origem;
} struct_message;

struct_message dadosEnviar;
struct_message dadosReceber;

bool estadoLED = LOW;

esp_now_peer_info_t peerInfo;  //Estrutura de configuração do receptor

// Funcao callback de envio
void esp_now_send_callback(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Exibe o status do envio
  Serial.print("Status do ultimo pacote enviado: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Enviado com Sucesso" : "Falha no envio");
}

// Funcao callback de recepcao
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  //imprime mensagem recebida
  Serial.print("Mensagem recebida de: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(":");
  }
  memcpy(&dadosReceber, incomingData, sizeof(dadosReceber));

  if (dadosReceber.origem == ID) return;

  Serial.println();
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Origem do ID: ");
  Serial.println(dadosReceber.origem);

  if (dadosReceber.botaoId == 2) {
    estadoLED = !estadoLED;
    digitalWrite(LED_VERDE, estadoLED);
  }
}

void setup() {
  Serial.begin(115200);  // Inicia a comunicação serial
  WiFi.mode(WIFI_STA);   // Configura o ESP32 como Station

  pinMode(LED_VERDE, OUTPUT);

  // Inicia o ESP-NOW
  if (esp_now_init() != ESP_OK) {                  // Verifica se o ESP-NOW foi iniciado com sucesso
    Serial.println("Erro ao iniciar o ESP-NOW");   // Exibe uma mensagem de erro
    return;                                        // Retorna ao loop
  }

  esp_now_register_send_cb(esp_now_send_callback); // Registra a função de callback de transmissao

  //Registra o receptor
  memcpy(peerInfo.peer_addr, macAddress, 6);        // Copia o endereço MAC do receptor
  peerInfo.channel = 0;                             // Configura o canal do ESP-NOW
  peerInfo.encrypt = false;                         // Configura a criptografia

  // Adiciona o receptor
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {        // Verifica se o receptor foi adicionado
    Serial.println("Falha ao adicionar o Receptor");  // Exibe uma mensagem de erro
    return;                                           // Retorna
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

void loop() {
  button.update();
  if (button.wasPressed()) {
    Serial.println("BOTAO PRESSIONADO");
    dadosEnviar.botaoId = 1;
    dadosEnviar.origem = ID;
    esp_now_send(macAddress, (uint8_t *)&dadosEnviar, sizeof(dadosEnviar));
  }
  delay(200);
}