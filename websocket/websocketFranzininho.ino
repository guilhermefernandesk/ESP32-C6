/*
    Placa Franzininho WiFi Lab01
*/

#include <WiFi.h>               // Inclui a biblioteca WiFi para conectar ao WiFi
#include <ESPAsyncWebServer.h>  // Inclui a biblioteca para servidor web assíncrono
#include <AsyncTCP.h>           // Inclui a biblioteca para TCP assíncrono
#include "html_page.h"          // Inclui o arquivo HTML que contém a página web
#include "credentials.h"        // Inclui o arquivo de credenciais do WiFi
#include <button.h>             // Inclui a biblioteca para manipulação de botões
#include <DHT.h>                // Inclusão da biblioteca DHT

//Mapa de pinos da Franzininho WiFi LAB01
#define LED_VERMELHO 14  // Pino do LED Vermelho
#define LED_VERDE 13     // Pino do LED Verde
#define LED_AZUL 12      // Pino do LED Azul
#define BOTAO_1 7        // Pino do Botão 1
#define BOTAO_2 6        // Pino do Botão 2
#define BOTAO_3 5        // Pino do Botão 3
#define BOTAO_4 4        // Pino do Botão 4
#define BOTAO_5 3        // Pino do Botão 5
#define BOTAO_6 2        // Pino do Botão 6
#define BUZZER 17        // Pino Do Buzzer
#define LDR 1
#define DHTPIN 15  // Definição do pino onde o DHT11 está conectado

#define DHTTYPE DHT11  // Definição do tipo do sensor (podemos escolher entre DHT11, DHT22 ou DHT21)

// Mapeamento dos pinos
const uint8_t LED_PIN = LED_VERDE;   // Define o pino do LED VERDE
const uint8_t BUTTON_PIN = BOTAO_1;  // Define o pino do botão 1
const uint8_t LDR_PIN = LDR;         // Define o pino do LDR (sensor de luz)
const uint8_t DHT_PIN = DHTPIN;      // Define o pino do DHT11

// Inicializa o sensor DHT
DHT dht(DHT_PIN, DHTTYPE);
// Cria um objeto Button para o botão como input pull-up
Button btn(BUTTON_PIN);

const uint32_t timeNotify = 2000;  // Tempo em milissegundos para notificar os clientes

bool statusLED = LOW;  // Status do LED  (LOW = Desligado / HIGH = Ligado)

uint8_t brilhoPWM = 0;  // Valor do brilho PWM inicial

// Credenciais de rede WiFi
const char *ssid = SSID;
const char *password = PASSWORD;

// Criação do servidor web e WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Evento WiFi IP
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("WiFi Conectado, IP:");
  Serial.println(WiFi.localIP());
}

// Evento WiFi Desconectado
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("Desconectado do WiFi");
  Serial.print("WiFi perdeu a conexão. Razão: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  Serial.println("Tentando reconectar");
  WiFi.begin(ssid, password);
}

// Função para notificar todos os clientes conectados com o estado atual
void notifyClients() {
  String msg = "LED=" + String(statusLED ? "on" : "off") + ";LDR=" + String(analogRead(LDR_PIN)) + ";TEMP=" + String(dht.readTemperature()) + ";UMID=" + String(dht.readHumidity()) + ";PWM=" + String(brilhoPWM);
  ws.textAll(msg);
}

void setPWM(int value) {
  brilhoPWM = value;
  setLED(true);
}
// Função para definir o estado do LED e notificar os clientes
void setLED(bool state) {
  statusLED = state;
  ledcWrite(LED_PIN, statusLED ? brilhoPWM : 0);  // Define o valor PWM para o LED vermelho
  notifyClients();
}

// Função para lidar com o pressionamento do botão
void handleButtonPress() {
  statusLED = !statusLED;
  setLED(statusLED);
  Serial.println("Botão pressionado, LED " + String(statusLED ? "ligado" : "desligado"));
}

// Função para lidar com eventos do WebSocket
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%lu connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      client->text("LED=" + String(statusLED ? "ON" : "OFF") + ";LDR=" + String(analogRead(LDR_PIN)));
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      {
        String msg = "";
        for (size_t i = 0; i < len; i++) {
          msg += (char)data[i];
        }
        Serial.println("Mensagem recebida via WebSocket: " + msg);
        if (msg == "on") setLED(true);
        else if (msg == "off") setLED(false);
        else if (msg.startsWith("PWM=")) setPWM(msg.substring(4).toInt());
        break;
      }
    default:
      break;
  }
}

void handleRoot(AsyncWebServerRequest *request) {
  Serial.println("Acessando a página principal");
  request->send(200, "text/html", index_html);  // Retorna o HTML para o cliente
}

void handleNotFound(AsyncWebServerRequest *request) {
  Serial.println("Página não encontrada");
  request->send(404, "text/plain", "Not found");  // Retorna uma resposta 404 Not Found
}

void setup() {
  Serial.begin(115200);
  pinMode(LDR_PIN, INPUT);

  ledcAttach(LED_PIN, 5000, 8);  // Configura o pino, frequência e resolução do PWM
  ledcWrite(LED_PIN, 0);         // Inicializa o PWM do LED com valor 0 (desligado)

  dht.begin();  // Inicia o sensor

  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);

  WiFi.disconnect(true);  // Desconecta de qualquer rede WiFi
  delay(1000);            // Aguarda 1 segundo

  // Registra os eventos do WiFi
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  //Conecta na rede WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  ws.onEvent(onWebSocketEvent);  // Registra o evento do WebSocket
  server.addHandler(&ws);        // Adiciona o WebSocket ao servidor

  // Definicao de Rotas do Servidor (como um Backend)
  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();  // Inicia o servidor web
  Serial.println("Servidor iniciado");
}

void loop() {
  // Notifica clientes a cada timeNotify milissegundos
  static uint32_t lastUpdate = 0;
  if (millis() - lastUpdate >= timeNotify) {
    lastUpdate = millis();
    notifyClients();
  }
  btn.update();  // Atualiza o estado do botão
  if (btn.wasPressed()) {
    handleButtonPress();  // Chama a função para lidar com o pressionamento do botão
  }
  // Limpa conexões WebSocket desconectadas
  ws.cleanupClients();
  delay(10);  // Pequena pausa para evitar sobrecarga do loop
}
