/*********************************************************
 * Projeto: Controle MQTT com ESP32-C6 + FreeRTOS
 *
 * Descrição: Este projeto permite publicar e receber mensagens MQTT
 *            para controle de um LED via broker público (Eclipse).
 *            Também utiliza FreeRTOS para tratar eventos de botão
 *            de forma assíncrona.
 *
 * Autor: Guilherme Fernandes
 * Data: 14/07/2025
 * Placa: ESP32-C6 DevKitC-1
 *********************************************************/
#include <WiFi.h>
#include <PubSubClient.h>
#include "credentials.h"
#include <button.h>  // Inclui a biblioteca para manipulação de botões
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Definição dos pinos
#define LED_VERMELHO 18
#define BOTAO 19

const uint8_t LED_PIN = LED_VERMELHO;  // Define o pino do LED
const uint8_t BUTTON_PIN = BOTAO;      // Define o pino do botão

bool ledState = false;  // Variável para armazenar o estado do LED

Button button(BUTTON_PIN);  // Inicializa o botao com input pull-up

// Configuração Wi-Fi
const char* ssid = SSID;          // SSID da rede Wi-Fi substituir pelo SSID da sua rede
const char* password = PASSWORD;  // Senha da rede Wi-Fi substituir pela senha da sua rede

// Configuração do Broker MQTT
const char* mqtt_server = "broker.emqx.io";  // Endereço do Broker MQTT - Se desejar utilizar outro, substitua este pelo endereço do seu broker
const int mqtt_port = 8084;                           // Porta do Broker MQTT - TCP 1883

// Definição dos Tópicos MQTT
const char* topic_led_vermelho = "esp32/led/vermelho";  // Tópico para controle do LED Vermelho - Substitua pelo tópico desejado se necessário

WiFiClient espClient;            // Cria um cliente Wi-Fi
PubSubClient client(espClient);  // Cria um cliente MQTT

// Protótipos das funções
void taskBotao(void* parameter);                                 // Função da task que lê e envia os dados do DHT11 via MQTT
void callback(char* topic, byte* message, unsigned int length);  // Função de callback para processar mensagens recebidas do MQTT

void setLED(bool state) {
  ledState = state;                                                             // Define o estado do LED
  digitalWrite(LED_PIN, ledState);                                              // Atualiza o estado do LED
  Serial.println("LED VERMELHO " + String(ledState ? "LIGADO" : "DESLIGADO"));  // Imprime o estado do LED no monitor serial
}


// Evento WiFi IP
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("\nWiFi Conectado!");  // Após conectar, imprime a mensagem no monitor serial
  Serial.print("Endereço de IP: ");     // Imprime a mensagem no monitor serial
  Serial.println(WiFi.localIP());       // Imprime o endereço IP no monitor serial
}

// Evento WiFi Desconectado
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("Desconectado do WiFi");
  Serial.print("WiFi perdeu a conexão. Razão: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  Serial.println("Tentando reconectar");
  WiFi.begin(ssid, password);
}

// Função de configuração do Wi-Fi
void setupWiFi() {
  // Registra os eventos do WiFi
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  WiFi.begin(ssid, password);              // Conecta-se à rede Wi-Fi
  Serial.print("Conectando ao WiFi...");   // Aguarda a conexão com a rede
  while (WiFi.status() != WL_CONNECTED) {  // Enquanto não estiver conectado, aguarda
    delay(500);                            // Aguarda 500ms
    Serial.print(".");                     // Imprime um ponto no monitor serial
  }
}

// Função de reconexão ao Broker MQTT
void connectMQTT() {
  while (!client.connected()) {                                                         // Enquanto o cliente não estiver conectado ao Broker MQTT
    Serial.println("Conectando ao MQTT...");                                            // Imprime a mensagem no monitor serial
    if (client.connect("esp32_guilherme")) {  // Tenta conectar ao Broker MQTT
      Serial.println("Conectado!");                                                     // Imprime a mensagem no monitor serial

      // Assinar os tópicos dos LEDs
      client.subscribe(topic_led_vermelho);

      // Imprime a mensagem no monitor serial
      Serial.println("Assinado nos tópicos: ");
      Serial.println(topic_led_vermelho);
    } else {
      Serial.print("Falha, código: ");  // Imprime a mensagem no monitor serial
      Serial.println(client.state());   // Imprime o código de erro no monitor serial
      delay(5000);                      // Aguarda 5 segundos e tenta novamente
    }
  }
}

// Callback para processar mensagens recebidas do MQTT
void callback(char* topic, byte* message, unsigned int length) {
  String msg = "";                  // Cria uma string para armazenar a mensagem
  for (int i = 0; i < length; i++)  // Percorre a mensagem recebida
  {
    msg += (char)message[i];  // Concatena os caracteres da mensagem
  }

  // Imprime a mensagem recebida no monitor serial - Log
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);
  Serial.print("Conteúdo: ");
  Serial.println(msg);

  // Verifica se a mensagem é para o LED vermelho
  if (strcmp(topic, topic_led_vermelho) == 0)  // Compara o tópico recebido com o tópico do LED Vermelho
  {
    if (msg == "ON") {          // Se a mensagem for "ON"
      setLED(true);             // Liga o LED Vermelho
    } else if (msg == "OFF") {  // Se a mensagem for "OFF"
      setLED(false);            // Desliga o LED Vermelho
    }
  }
}

void setup() {
  Serial.begin(115200);  // Inicializa a comunicação serial
  setupWiFi();           // Configura a conexão Wi-Fi

  // Configuração dos LEDs como saída e desliga os LEDs
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  client.setServer(mqtt_server, mqtt_port);  // Define o servidor MQTT
  client.setCallback(callback);              // Define a função de callback
  connectMQTT();                             // Conecta ao Broker MQTT

  // Criando a Task para leitura e envio do botao
  xTaskCreate(
    taskBotao,     // Nome da função da task
    "Task Botao",  // Nome descritivo
    4096,          // Tamanho da stack
    NULL,          // Parâmetro (não usado)
    1,             // Prioridade da task
    NULL           // Handle (não necessário aqui)
  );
}

void loop() {
  if (!client.connected()) {
    connectMQTT();  // Se a conexão com o Broker MQTT foi perdida, reconecta
  }
  client.loop();  // Mantém a conexão com o Broker MQTT
}

/*-------------------------------------------------------------------------------
  Tarefa para ler o estado do botão e enviar para a fila
 *---------------------------------------------------------------------------*/
void taskBotao(void* parameter) {
  for (;;) {
    button.update();                                           // Atualiza o estado do botão
    if (button.wasPressed()) {                                 // Verifica se o botão foi pressionado
      Serial.println("Botão pressionado");                     // Imprime no monitor serial
      ledState = !ledState;                                    // Inverte o estado do LED
      String msg = ledState ? "ON" : "OFF";                    // Mensagem para
      client.publish(topic_led_vermelho, msg.c_str(), false);  // Publica a mensagem no tópico do LED Vermelho
    }
    vTaskDelay(pdMS_TO_TICKS(100));  // Aguarda 100 ms antes de verificar novamente
  }
}
